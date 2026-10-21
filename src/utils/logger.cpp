#include <resources.hpp>
#include <filesystem>
#include <logger.hpp>
#include <utils.hpp>

using namespace Engine::Logging;

void Logger::openLog() {
    if (s_outFile.is_open()) return;
    
    std::filesystem::path path = ResourceManager::getBinaryDir() / logger_defaultDir;
        if (!std::filesystem::is_directory(path)) {
            std::filesystem::create_directory(path);
        }
        path = path / ("output-" + Time::timeStampForFile() + ".log");

    s_outFile.open(path, std::ios::app);
    if (s_outFile.is_open()) {
        s_outFile << "DEBUG LOG FILE\nDATE: " << Time::timeStampForFile() << "\nPLATFORM: " << Utils::getPlatform() << std::endl << std::endl;
    }
}

void Logger::closeLog() {
    if (s_outFile.is_open()) {
        get().logInfo("Writing log...");
        s_outFile.close(); 
    }
}

Logger::Logger(std::string name){
    m_logName = name;
    openLog();
}

const std::string Logger::getSeverityPrefix(LogSeverity &sv) {
    std::string strPrefix;
    switch (sv) {
        case LogSeverity::Error: {
            strPrefix = "[ERR!]";
            break;
        }
        case LogSeverity::Warning: {
            strPrefix = "[WARN]";
            break;
        }
        case LogSeverity::Info: {
            strPrefix = "[INFO]";
            break;
        }
        default: {
            strPrefix = "      ";
            break;
        }
    }
    return strPrefix;
}