#pragma once
#include <iostream>
#include <string>

namespace Engine {

    class Utils {
    public: 
    // this program will never be ported to all these platforms but like whatever
        static std::string inline getPlatform() {
            #if defined(_WIN32) || defined(_WIN64)
                return "Windows";
            #elif defined(__APPLE__) || defined(__MACH__)
                #include <TargetConditionals.h>
                #if TARGET_OS_IPHONE
                    return "iOS";
                #else
                    return "macOS";
                #endif
            #elif defined(__ANDROID__)
                return "Android";
            #elif defined(__linux__)
                return "Linux";
            #elif defined(__FreeBSD__)
                return "FreeBSD";
            #else
                return "Unknown Platform";
            #endif
        }
    };
}