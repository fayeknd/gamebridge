#include <time.hpp>
#include <gl_context.hpp>

using namespace Engine;

Time::Timer Time::s_deltaTimer = {};

// deltatime getter
double Time::deltaTime() {
    return s_deltaTime;
}
// get the value of the delta timer right now
double Time::deltaTimeNow() {
    return s_deltaTimer.count();
}

// return the time since program startup
double Time::sinceStartup() {
    // just return glfwGetTime lol it already does this
    return glfwGetTime();
}

// return the timer count
double Time::Timer::count() {
    if (!m_counting && !m_hasStarted) return 0;
    std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - m_start;
    return elapsed.count() - m_pauseDur;
}

// start the timer 
void Time::Timer::start() {
    // if already counting, return early
    if (m_counting) return;

    // if this is the first start, set the time to now.
    if (!m_hasStarted) m_start = std::chrono::system_clock::now();
    else {
        // if m_hasStarted is true, the timer has been paused.
        // This line just calculates how long it was paused for
        // (this is subtracted from count())
        // set the duration the timer was paused for
        m_pauseDur += (timeNow() - m_end).count();
    }
    // set hasStarted and counting to true.
    m_hasStarted = true;
    m_counting = true;
}

// stop the timer
void Time::Timer::stop() {
    // set the m_end time to now.
    m_end = std::chrono::system_clock::now();
    m_counting = false;
}

// reset the timer
void Time::Timer::reset() {
    // reset hasStarted
    m_hasStarted = false;
    // pause duration is 0
    m_pauseDur = 0; 
    // if the timer was counting when 
    // reset was called, resume the timer.
    if (m_counting) {
        m_counting = false;
        start();
    }
}

// reset the delta timer
void Time::_resetDeltaTimer() { 
    // set delta time value
    s_deltaTime = s_deltaTimer.count();
    // restart the delta timer
    s_deltaTimer.reset();
}