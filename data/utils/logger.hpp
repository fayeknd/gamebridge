#pragma once
#include <format>
#include <time.hpp>
#include <filesystem>
#include <vector>
#include <memory>
#include <fstream>

#pragma region colourcodes
// taken from https://github.com/fayeknd/Application/blob/b93812cc1c5f18045dbb9754be682b88c0b5b095/Application/DebugColours.h
// might use these at some point
#define DBG_RESET               "\033[0m"
#define DBG_BLACK               "\033[30m"             // Black
#define DBG_RED                 "\033[31m"             // Red
#define DBG_GREEN               "\033[32m"             // Green
#define DBG_YELLOW              "\033[33m"             // Yellow
#define DBG_BLUE                "\033[34m"             // Blue
#define DBG_MAGENTA             "\033[35m"             // Magenta
#define DBG_CYAN                "\033[36m"             // Cyan
#define DBG_WHITE               "\033[37m"             // White
#define DBG_BOLDBLACK           "\033[1m\033[30m"      // Bold Black
#define DBG_BOLDRED             "\033[1m\033[31m"      // Bold Red
#define DBG_BOLDGREEN           "\033[1m\033[32m"      // Bold Green
#define DBG_BOLDYELLOW          "\033[1m\033[33m"      // Bold Yellow
#define DBG_BOLDBLUE            "\033[1m\033[34m"      // Bold Blue
#define DBG_BOLDMAGENTA         "\033[1m\033[35m"      // Bold Magenta
#define DBG_BOLDCYAN            "\033[1m\033[36m"      // Bold Cyan
#define DBG_BOLDWHITE           "\033[1m\033[37m"      // Bold White

#pragma endregion colourcodes

namespace Engine {
    
    namespace Logging {

        enum class LogSeverity {
            None        = 0x00,
            Warning     = 0x01,
            Error       = 0x02,
            Info        = 0x03,
        };

        constexpr const char* logger_emptyName = "None";
        constexpr const char* logger_defaultDir = "logs";
        
        class Logger { 
        protected:
        
            static inline std::ofstream s_outFile; 

        public:

            static Logger& get() {
                static Logger instance("DEBUG");
                return instance;
            }

            #define DBG Logging::Logger::get()

            // unsupported
            std::string m_logCol  = DBG_WHITE;
            std::string m_logName = logger_emptyName;
            
            static void openLog();
            static void closeLog();

            Logger(std::string name);

            const std::string getSeverityPrefix(LogSeverity& sv);

            template <typename... Args> 
            void log(LogSeverity sv, std::string str, Args&& ... args) {
                std::stringstream ln;
                std::string logName = (m_logName != logger_emptyName) ? (" [" + m_logName + "] ") : "";

                ln << "[" << Time::timeStamp() << "] " << getSeverityPrefix(sv) << logName << std::vformat(str, std::make_format_args(args...)) << "\n";

                std::string logStr = ln.str();

                std::cout << logStr;

                if (s_outFile.is_open()) {
                    s_outFile << logStr;
                }
            }
            
            template <typename... Args>
            void logWarning(std::string str, Args&& ... args) { log(LogSeverity::Warning, str, args...); }

            template <typename... Args>
            void logError(std::string str, Args&& ... args) { log(LogSeverity::Error, str, args...); }

            template <typename... Args>
            void logInfo(std::string str, Args&& ... args) { log(LogSeverity::Info, str, args...); }

        };
    }
}