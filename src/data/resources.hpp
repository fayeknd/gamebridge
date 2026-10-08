#pragma once

#include <zip.h>
#include <filesystem>
#include <unordered_map>
#include <memory.h>
#include <iostream>
#include <fstream>
#include <vector>

namespace Engine {

    class Archive {
    protected:

        zip* m_zip;
        std::string m_arPath;

    public:

        const std::string& getPath();
        static std::shared_ptr<Archive> getArchive(const std::string& path); 
        ~Archive();

        friend class ResourceManager;
    };

    // resources=>sprites/awesomesprite
    constexpr const char* resourceManagerSeparator = "=>";
    constexpr unsigned int resourceManagerSeparatorLen = 2;

    class ResourceManager {
    protected:

        static inline std::filesystem::path s_binaryDir = "none";

    public:

        static inline std::unordered_map<std::string, std::shared_ptr<Archive>> s_archiveCache;
        static const std::filesystem::path& getBinaryDir();
        static void freeArchives();
        static std::shared_ptr<Archive> getArchiveInPath(const std::string& path, std::string* outPath);
        static std::string getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path);
        static std::vector<unsigned char> loadRaw(const std::string& path, bool keepArchive = true);

    };

}