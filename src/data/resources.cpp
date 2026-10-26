#include "cachable.hpp"
#include <resources.hpp>
#include <vector>

#ifdef _WIN32
#include <windows.h>    //GetModuleFileNameW
#else
#include <limits.h>
#include <unistd.h>     //readlink
#endif

using namespace Engine;

// returns the archive path.
const std::string& Archive::getPath() {
    return m_arPath;
}

// returns an archive from a provided archive path.
// if the archive is not cached, load an archive and cache it.
std::shared_ptr<Archive> Archive::getArchive(const std::string& path) {

    // attempt to find an already present archive.
    auto iterator = ResourceManager::s_archiveCache.find(path);
    if (iterator != ResourceManager::s_archiveCache.end()) {
        // found an archive. return that!
        return ResourceManager::s_archiveCache[path];
    }

    // no archive found, cached, load a new one.
    auto ar = std::shared_ptr<Archive>(new Archive);

    // attempt to open a zip archive with the provided path
    // we don't have to write to this, so we only need to
    // open it as readonly. (ZIP_RDONLY)
    int err;
    ar->m_zip = zip_open(path.c_str(), ZIP_RDONLY, &err);

    // zip load failed?
    if (!ar->m_zip) {
        // print the error code provided by zip_open and return a nullptr
        ResourceManager::log().logError("Archive '{}' loading failed. Error code:", path, err);
        return nullptr;
    }
    // no error. set the ar path to the function provided path
    ar->m_arPath = path;

    // add the archive to the archive cache.
    ResourceManager::s_archiveCache.emplace(ar->m_arPath, ar);
    return ar;
}

// deconstructor
Archive::~Archive() {
    // make sure to close the zip file.
    zip_close(m_zip);
}

// free all archives from memory.
void ResourceManager::freeArchives() {
    s_archiveCache.clear();
}

// Returns the path to the program executable.
// if s_binaryDir is "none", it will compute the 
// path. otherwise, just return s_binaryDir.
const std::filesystem::path& ResourceManager::getBinaryDir() {
    
    // if the s_binaryDir has been set already, just return that.
    if (s_binaryDir != "none") return s_binaryDir;

    // s_binaryDir is none, so we have to set it.

#ifdef _WIN32
    // passing null through to GetModuleFileNameW
    // should apparently get the exe path.
    // (idk if this works, please test this.)
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    s_binaryDir = path;
#else
    // theres probably a way to do this but on Renderers.
    s_binaryDir = std::filesystem::canonical("/proc/self/exe");
#endif
    
    // we have the full path of the binary, but we need the path
    // of the directory the binary is in. so just get the binary parent
    // path.
    s_binaryDir = s_binaryDir.parent_path();
    return s_binaryDir;
}

// attempts to return / load a new (if none exists in memory)
// archive that is present in memory.
// for instance, "archive=>some/asset/here"
// will attempt to load an archive titled "archive".
// returns nullptr if failed.
std::shared_ptr<Archive> ResourceManager::getArchiveInPath(const std::string &path, std::string* outPath) {
    
    // get the first instance of the separator found in the string.
    size_t index = path.find_first_of(resourceManagerSeparator);
    if (index == std::string::npos) {
        // no instance of the separator in the string.
        return nullptr; 
    }

    // get the archive full path
    std::string arStr = getBinaryDir().string() + '/' + path.substr(0, index);

    // get the archive from the cache or force load it.
    auto ar = Archive::getArchive(arStr);

    // if no archive, file path was probably incorrect.
    if (!ar) return nullptr;

    // otherwise, set the outPath to the substr of the path var
    // after the separator. this is what the separatorLen const was for.
    *outPath = path.substr(index + resourceManagerSeparatorLen, path.length());
    return ar;
}

// attempt to load a file from a provided archive.
// usually you should not have to use this.
bytes ResourceManager::getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path) {

    // path as a raw char string
    const char* pCstr = path.c_str();

    // create a container for the file info
    zip_stat_t fStat;
    zip_stat_init(&fStat);

    // attempt to retrieve the file info
    if (zip_stat(archive->m_zip, pCstr, 0, &fStat) != 0) {
        // file not found in archive.
        log().logError("File '{}' not found in archive '{}'.", path, archive->getPath());
        // return an empty vector constructor.
        return {};
    }

    // file info retrieved just fine. open the file.
    // note: this is not opening the archive itself, 
    // just the file.
    zip_file* file = zip_fopen(archive->m_zip, pCstr, 0);
    if (!file) {
        // file failed to open.
        log().logError("Failed to open file '{}' in archive '{}'", path, archive->getPath());
        // close the file, we don't need it in memory anymore.
        zip_fclose(file);
        return {};
    }

    // create our uchar buffer to store file data
    // we already know how big it's going to be
    // from the file info (fstat)
    bytes buf(fStat.size);

    // read the raw bytes of the file into the buf vector.
    zip_int64_t readBytes = zip_fread(file, buf.data(), fStat.size);

    // we have read the data, we can close the file.
    zip_fclose(file);

    if (readBytes <= 0) {
        // readBytes is 0 (or less than), this shouldn't be.
        // something has gone wrong, or the file has no size.
        log().logError("Error reading file '{}' in archive '{}'.", path, archive->getPath());
        return {};
    }

    return buf;

}


// most abstracted method. provide a string such as "ar=>file.txt"
// and this will return file.txt as a vector of uchars.
// if you're only going to be loading a single file from
// the archive, you can set keepArchive to false. this will
// unload the archive immediately. 
bytes ResourceManager::loadRaw(const std::string& path, bool keepArchive) {

    // create a container for the "real" path
    std::string _internalPath;
    // get an archive from the path
    auto archive = getArchiveInPath(path, &_internalPath);

    // archive is null, return.
    if (!archive) {
        return {};
    }

    // read the bytes from the file in the archive.
    bytes buf = getFileFromArchive(archive, _internalPath);

    if (!keepArchive) {
        // if keepArchive is false, unload the archive from memory.
        s_archiveCache.erase(archive->m_arPath);
    }

    // return the read data.
    return buf;
}