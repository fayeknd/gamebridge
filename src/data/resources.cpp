#include "resources.hpp"
#include <vector>

#ifdef _WIN32
#include <windows.h>    //GetModuleFileNameW
#else
#include <limits.h>
#include <unistd.h>     //readlink
#endif

using namespace Engine;

const std::string& Archive::getPath() {
    return m_arPath;
}

std::shared_ptr<Archive> Archive::getArchive(const std::string& path) {

    auto iterator = ResourceManager::s_archiveCache.find(path);
    if (iterator != ResourceManager::s_archiveCache.end()) {
        return ResourceManager::s_archiveCache[path];
    }

    auto ar = std::shared_ptr<Archive>(new Archive);
    int err;
    ar->m_zip = zip_open(path.c_str(), ZIP_RDONLY, &err);
    if (!ar->m_zip) {
        std::cerr << "Archive '" << path << "' loading failed. Error code: " << err << std::endl;
        return nullptr;
    }
    ar->m_arPath = path;
    ResourceManager::s_archiveCache.emplace(ar->m_arPath, ar);
    return ar;
}

Archive::~Archive() {
    zip_close(m_zip);
}

void ResourceManager::freeArchives() {
    s_archiveCache.clear();
}

const std::filesystem::path& ResourceManager::getBinaryDir() {
    
    if (s_binaryDir != "none") return s_binaryDir;

#ifdef _WIN32
    wchar_t path[MAX_PATH] = { 0 };
    GetModuleFileNameW(NULL, path, MAX_PATH);
    s_binaryDir = path;
#else
    s_binaryDir = std::filesystem::canonical("/proc/self/exe");
#endif
    
    s_binaryDir = s_binaryDir.parent_path();
    return s_binaryDir;
}

std::shared_ptr<Archive> ResourceManager::getArchiveInPath(const std::string &path, std::string* outPath) {
    
    size_t index = path.find_first_of(resourceManagerSeparator);

    std::string arStr = getBinaryDir().string() + '/' + path.substr(0, index);

    auto ar = Archive::getArchive(arStr);
    if (!ar) return nullptr;

    *outPath = path.substr(index + resourceManagerSeparatorLen, path.length());
    return ar;
}

std::string ResourceManager::getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path) {

    const char* pCstr = path.c_str();

    zip_stat_t fStat;
    zip_stat_init(&fStat);

    if (zip_stat(archive->m_zip, pCstr, 0, &fStat) != 0) {
        std::cerr << "File '" << path << "' not found in archive '" << archive->getPath() << "'." << std::endl;
        return nullptr;
    }

    zip_file* file = zip_fopen(archive->m_zip, pCstr, 0);
    if (!file) {
        std::cerr << "Failed to open file '" << path << "' in archive '" << archive->getPath() << "'." << std::endl;
        zip_fclose(file);
        return nullptr;
    }

    std::vector<char> buf(fStat.size);
    zip_int64_t readBytes = zip_fread(file, buf.data(), fStat.size);
    zip_fclose(file);
    if (readBytes <= 0) {
        std::cerr << "Error reading file '" << path << "' in archive '" << archive->getPath() << "'." << std::endl;
        return nullptr;
    }

    std::string data(buf.begin(), buf.end());

    return data;

}



std::vector<unsigned char> ResourceManager::loadRaw(const std::string& path, bool keepArchive) {
    std::string file = "";
    std::string _internalPath;
    auto archive = getArchiveInPath(path, &_internalPath);
    if (archive) {
        file = getFileFromArchive(archive, _internalPath);
    }

    if (!keepArchive) {
        s_archiveCache.erase(archive->m_arPath);
    }

    std::vector<unsigned char> buf(file.begin(), file.end());
    buf.push_back('\0'); // \0 is the null terminator for strings
    return buf;
}