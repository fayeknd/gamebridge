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
        
        static inline std::unordered_map<std::string, std::shared_ptr<Archive>> s_archiveCache;

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

        std::filesystem::path m_binaryDir = "none";

    public:
        const std::filesystem::path& getBinaryDir();
        static ResourceManager& get() {
            static ResourceManager instance;
            return instance;
        }

        void unloadArchives();
        std::shared_ptr<Archive> getArchiveInPath(const std::string& path, std::string* outPath);
        std::string getFileFromArchive(std::shared_ptr<Archive> archive, const std::string& path);
        std::vector<unsigned char> loadRaw(const std::string& path, bool keepArchive = true);

    };

}