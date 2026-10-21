#pragma once

#include <zip.h>
#include <filesystem>
#include <unordered_map>
#include <memory.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <logger.hpp>

namespace Engine {

    // this doesn't do anything except prevent having to type
    // std::vector<unsigned char> often.
    // it's just an alias.
    using bytes = std::vector<unsigned char>;

    // the Archive is basically our version of Unity's AssetBundle.
    // but because of how resources get loaded, you won't need to
    // interact with archives at all.

    class Archive {
    protected:

        // zip from libzip, the Archive can be treated as a 
        // "wrapper" for these zips. 
        zip* m_zip;

        // archive file path on the disk, for caching.
        std::string m_arPath;

    public:

        // returns the archive path.
        const std::string& getPath();

        // returns an archive from a provided archive path.
        // if the archive is not cached, load an archive and cache it.
        static std::shared_ptr<Archive> getArchive(const std::string& path); 
        
        // basic deconstructor
        ~Archive();

        // basically give the ResourceManager trusted access to this class'
        // protected variables, but no other class.
        friend class ResourceManager;
    };

    // just a constexpr for the separator to look for in path strings.
    // In this case, given an archive is named simply "ar" in our files,
    // the path to an image titled "image.png" under "images" in that archive 
    // would look like this:
    // "ar=>images/image.png" 
    constexpr const char* resourceManagerSeparator = "=>";

    // the length of the above separator. can be computed at runtime
    // but  there's no point and it takes up like half a bit of memory so
    constexpr unsigned int resourceManagerSeparatorLen = 2;


    // the principle i have gone with for this is to keep every archive
    // loaded via the ResourceManager in memory until calling freeArchives().

    // The reason it's this way is because constantly loading an archive into 
    // memory just to get one object from it and then unload that archive, 
    // just to potentially require more objects from that archive is expensive.

    // so archives remain loaded, so access to their objects is quick,
    // and then when we are sure we don't need to be loading anything else
    // at least for a while, we can call freeArchives() to remove them
    // from memory.

    // This is not a flawless design, and there might be issues if we are loading
    // gigabytes of items, but until then we should be okay.


    // the ResourceManager is a static class responsible for the loading
    // and unloading of any sort of data stored in an archive, 
    // and general management for archives.

    // it is not a singleton with a get() function, simply because
    // it feels wrong to type ResourceManager::get().loadRaw();

    class ResourceManager {
    protected:

        // stores the path to the program executable.
        // computed once and then stored permanently in this.
        // this would be a const, but because it has to be
        // computed at runtime it can't be.

        static inline std::filesystem::path s_binaryDir = "none";

        static Logging::Logger& log() {
            static Logging::Logger instance("RESOURCE");
            return instance;
        }

        friend class Archive;

    public:

        // stores all the loaded archives. Uses their file path as the key.
        static inline std::unordered_map<std::string, std::shared_ptr<Archive>> s_archiveCache;

        // Returns the path to the program executable.
        // if s_binaryDir is "none", it will compute the 
        // path. otherwise, just return s_binaryDir.
        static const std::filesystem::path& getBinaryDir();

        // free all the archives from memory.
        static void freeArchives();

        // attempts to return / load a new (if none exists in memory)
        // archive that is present in memory.
        // for instance, "archive=>some/asset/here"
        // will attempt to load an archive titled "archive".
        // returns nullptr if failed.

        // path is the path provided, outPath is the 
        // new path with the "archive=>" part removed.
        // Usage looks like:
        // std::string newPath;
        // auto ar = ResourceManager::getArchiveInPath(fullPath, &newPath);
        // if (!ar) { error }
        // else ( dostuff )
        static std::shared_ptr<Archive> getArchiveInPath(const std::string& path, std::string* outPath);
        
        // attempt to load a file from a provided archive. returns an unsigned char vector.
        // usually you should not have to use this.
        static bytes getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path);

        // most abstracted method. provide a string such as "ar=>file.txt"
        // and this will return file.txt as a vector of uchars.
        // if you're only going to be loading a single file from
        // the archive, you can set keepArchive to false. this will
        // unload the archive immediately. 
        static bytes loadRaw(const std::string& path, bool keepArchive = true);

    };

}