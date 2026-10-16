#pragma once
#include <iostream>
#include <stdio.h>
#include <chrono>
#include <ctime>

namespace Engine {

    // inspired by another implementation of mine: https://github.com/fayeknd/peachmilk_editor/blob/main/src/system/time.hpp. 
    // i'm not very confident with time stuff or really anything to do with chrono, so i'm not sure this is extremely accurate.

    // a static class that stores general program time information.
    // stores the delta time
    class Time {

        // track the frame delta time
        static inline double s_deltaTime = 0;

        // define the timer class
        class Timer {
        private:  
            // is timer counting?
            bool m_counting;

            // has the timer started?
            bool m_hasStarted = false;

            // start point of the timer
            std::chrono::system_clock::time_point m_start;
            
            // end point of the timer
            std::chrono::system_clock::time_point m_end;

            // the duration the timer has been paused
            double m_pauseDur = 0;

            // get the timer elapsed duration (helper function)
            std::chrono::system_clock::time_point timeNow() {
                // current time point
                std::chrono::system_clock::time_point t;
                
                // if we are currently counting up, return the time now.
                // else, retuen the time ended at.
                t = (m_counting) ? std::chrono::system_clock::now() : m_end;
                return t;
            }

        public:

            // return the timer count
            double count();

            // return if the timer is currently counting or not 
            bool isCounting() { return m_counting; }

            // start the timer
            void start();

            // stop the timer
            void stop();

            // reset the timer
            void reset();
        } static s_deltaTimer; // the delta timer!!

        // reset the delta timer 
        static void _resetDeltaTimer();

        friend class Window;
        
    public:

        // deltatime getter
        static double deltaTime();
        // get the value of the delta timer right now
        static double deltaTimeNow();

        // get the time since the program begun
        static double sinceStartup();

    };

}