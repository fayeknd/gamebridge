#include "cachable.hpp"
#include <window.hpp>

#include <GLFW/glfw3native.h>
#include <resources.hpp>
#include <shader.hpp>
#include <entity.hpp>
#include <component/main_comp.hpp>
#include <time.hpp>
#include <component/camera.hpp>
#include <numeric>
#include <input.hpp>

using namespace Engine;

// returns the glfw window handle. (different from the HWND window handle.)
GLFWwindow* Window::getWndHandle() const {
    return m_wnd;
}

// returns the window size.
fm::uvec2 Window::getWndSize() {
    return m_wndSize;
}

// sets the window size via glfwSetWindowSize(m_wnd, v.x, v.y).
void Window::setWndSize(fm::uvec2 v) {
    m_wndSize = v;
    if (GLContext::get().isGLReady()) glfwSetWindowSize(m_wnd, v.x, v.y);
}

// returns the window title. 
std::string Window::getWndTitle() {
    return m_wndTitle;
}

// sets the window title via glfwSetWindowTitle(m_wnd, s.c_str()).
void Window::setWndTitle(std::string s) {
    m_wndTitle = s;
    if (GLContext::get().isGLReady()) glfwSetWindowTitle(m_wnd, s.c_str());
}

// returns the current window fullscreen state.
FullscreenMode Window::getFullscreenMode() {
    return m_fsMode;
}

// sets the window state via a switch statement. 
void Window::setFullscreenMode(FullscreenMode fm) {

    // if the current FullscreenMode is already this, just return early.
    if (fm == m_fsMode) return; 
    switch (fm) {
        case FullscreenMode::Fullscreen: { // swap to fullscreen 

            // get current width and height
            int w, h;
            glfwGetWindowSize(m_wnd, &w, &h);
            
            // cache width and height to old size
            m_wndSize_old = fm::uvec2(w, h);

            // get current x and y position of the window
            int x, y;
            glfwGetWindowPos(m_wnd, &x, &y);

            // cache the x and y.
            m_wndPos_old = fm::uvec2(x, y);

            // get the monitor video mode for the width and height and refresh of the main monitor to fullscreen to.
            const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            // set the window to fullscreen on the primary monitor with the OS video mode current options.
            glfwSetWindowMonitor(m_wnd, glfwGetPrimaryMonitor(), 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate); 
            break; }

        case FullscreenMode::Windowed: { // swap to windowed
            
            // if swapping from fullscreen, use the cached old variables to restore window to original size and pos.
            if (m_fsMode == FullscreenMode::Fullscreen) {

                glfwSetWindowMonitor(m_wnd, NULL, m_wndPos_old.x, m_wndPos_old.y, m_wndSize_old.x, m_wndSize_old.y, 0);
                break;
            }

            // GLFW_DECORATED determines if the window borderless or not.
            // true means it is not borderless, false means it is.
            // it could be set to false, so change it back just in case.

            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_TRUE);
            break; } 

            // Don't know if this works on windows, should do
        case FullscreenMode::BorderlessWindowed: { // swap to borderless windowed

            // this isn't needed on windows but it is on linux. hide the window to make quick changes then show it again.
            glfwHideWindow(m_wnd);

            // set the decorated attrib
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_FALSE);

            // update window
            glfwPollEvents();

            // show the window again
            glfwShowWindow(m_wnd);

            // make sure it's focused
            glfwFocusWindow(m_wnd);
            
            // this is borderless windowed, so make sure the window is not maximised. 
            // glfwMaximiseWindow() is named weirdly. if the window is maximised when this is run,
            // it will un-maximise the window.
            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_FALSE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; }

        case FullscreenMode::Borderless: {

            // same as above, just read that
            glfwHideWindow(m_wnd);
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_FALSE);
            glfwPollEvents();
            glfwShowWindow(m_wnd);
            glfwFocusWindow(m_wnd);

            // also same as before, but check for the other case.
            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_TRUE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; } 
    }
    // make sure the window fullscreenmode is equal to the new fullscreenmode.
    m_fsMode = fm;
}

// get the target fps
uint16_t Window::getTargetFPS() {
    return m_targetFramesPerSecond;
}

uint16_t Window::getFPS() {
    return m_currentFramesPerSecond;
}

// get the target frame delta
double Window::getTargetFrameDelta() {
    return m_targetFrameDelta;
}

// set the target fps.
void Window::setTargetFPS(uint16_t fps) {
    m_targetFramesPerSecond = fps;
    // also calculate the new target delta.
    m_targetFrameDelta = static_cast<double>(1) / static_cast<double>(fps);
}

// set the target delta time.
void Window::setTargetFrameDelta(double delta) {
    m_targetFrameDelta = delta;
    // also calculate the new target fps.
    m_targetFramesPerSecond = static_cast<double>(1) / static_cast<double>(delta);
}

// get the current vsync mode
VSyncMode Window::getVSyncMode() {
    return m_vsyncMode;
}

// set the current vsync mode
void Window::setVSyncMode(VSyncMode vsm) {
    m_vsyncMode = vsm;
    glfwSwapInterval(static_cast<int>(vsm));
}

fm::uvec2 Window::getFractionalAspectRatio() {
    // get the greatest common divisor
    int div = std::gcd(m_wndSize.x, m_wndSize.y);
    // get ratio width
    uint8_t rWidth  = m_wndSize.x / div;
    // get ratio height
    uint8_t rHeight = m_wndSize.y / div;
    // awesome!
    return {rWidth, rHeight};
}

double Window::getAspectRatio() {
    // width / height = aspect
    return static_cast<double>(m_wndSize.x) / static_cast<double>(m_wndSize.y);
}

// creates the window. this should be the first function to run.
// a run through of what the parameters defaults are for:

// size: the window size. leave at -1 for the default value of 800x800. 
// title: window title. -1 for default also.
// fsMode: fullscreen mode. by default is just windowed.
// monitor: leave null for now.
// share: i don't really know what this does, it just gets passed through to glfwCreateWindow.
// extra_window_hints: a lambda function to add your own window hints in. create a function that
// has glfwWindowHint() functions and pass that function through as a parameter. not required.
int Window::createWindow(fm::uvec2 size, std::string title, FullscreenMode fsMode, GLFWmonitor *monitor, GLFWwindow *share, void(*extra_window_hints)()) {

    // if opengl is ready for use, the window has already been created. return early.
    if (GLContext::get().isGLReady()) return ERR_CONTEXT_EXISTS;

    // if the glfw context creation failed, also return early.
    if (GLContext::get().createGLFWContext(_default_window_hints, extra_window_hints) != GL_CREATE_CONTEXT_SUCCESS)
        return ERR_WINDOW_CREATION_FAILED;

    // if for whatever reason the m_wnd has already been assigned, return early. 
    if (m_wnd) return ERR_WINDOW_CREATION_FAILED;

    // if the root entity is null, something is wrong. return.
    if (!Entity::s_root) {
        return ERR_NO_ROOT_ENTITY;
    }

    // check for default parameter values and set them to the true defaults accordingly.
    // the reason they are -1 is to allow setting defaults to be done with the original variables
    // in window.hpp instead.
    if (size == fm::uvec2(-1)) size = m_wndSize;
    if (title == "-1") title = m_wndTitle;
    
    // actually create the window. unsure what share does.
    m_wnd = glfwCreateWindow(size.x, size.y, title.c_str(), monitor, share);

    // set the window variables to the passed through variables
    m_wndSize = size;
    m_wndTitle = title;
    
    // if glfwCreateWindow failed, return.
    if (!m_wnd) {
        return ERR_WINDOW_CREATION_FAILED;
    }

    // make sure the internal opengl context is tied to this window
    glfwMakeContextCurrent(m_wnd);

    // set the callback that is called whenever the window changes size.
    glfwSetFramebufferSizeCallback(m_wnd, _framebuffer_resize_callback);

    // Initializes the input class
    Input::init(m_wnd);

    // if GLAD initialisation fails, return.
    if (GLContext::get().createGLADContext() != GLAD_LOAD_SUCCESS) {
        return ERR_WINDOW_CREATION_FAILED;
    }
    
    // ensure the glViewport is set to the window size properly.
    glViewport(0, 0, size.x, size.y);
    m_viewportDirty = true;

    // set fullscreen mode passed in
    setFullscreenMode(fsMode);

    // set a default target fps
    setTargetFPS(60);
    
    // set the vsync mode
    setVSyncMode(VSyncMode::Off);

    // use the default renderdata
    GLContext::get().glState.renderData.useRenderData();

    // add the main entrypoint component to the root entity.
    Entity::s_root->addComponent<MainComponent>();

    // window creation succeeded!!
    return WINDOW_CREATION_SUCCESS;
}

// this function defines what glfw window hints are set by default.
// i.e glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
// this is passed through to the GLContext on window creation as a lambda.
void Window::_default_window_hints() {

    // set the opengl version. in our case, OpenGL 4.6.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    // we are using the core profile. the compatability profile is really weird and old.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // doesn't really need to be done, but just specify the colour bits used for the
    // glfw main framebuffer.
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    // only required on linux. forces the window to run under wayland instead of x11 (if available).
    #ifndef _WIN32
    glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    #endif    

    // set transparent framebuffer to true for obvious reasons.
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
}

// handles changing the m_wndSize value and glViewport.
void Window::_framebuffer_resize_callback(GLFWwindow* wnd, int w, int h) {

    // keep the viewport up to date
    glViewport(0, 0, w, h);
    get().m_viewportDirty = true;

    // make sure m_wndSize is aware of the updated size
    Window::get().m_wndSize = fm::vec<unsigned int>(w,h);
}

// begins the program rendering and logic loop. in main(), this should come last unless you have
// any cleanup stuff to perform after the program stops.
void Window::beginGameLoop() {

    Time::s_deltaTimer.start();

    // while glfw believes the window should not close
    while (!glfwWindowShouldClose(m_wnd)) {

        // if the delta timer is less than the target frame delta, 
        // then the frame was finished early. keep going until the 
        // delta timer is less than or equal to the target delta.
        // continue keyword means it skips everything else in the loop.

        if (Time::deltaTimeNow() < m_targetFrameDelta) {
            continue;
        }
        auto cam = Camera::activeCamera();
        // reset the delta timer at the start of this frame
        Time::_resetDeltaTimer();
        // and get the delta time
        double dt = Time::deltaTime();
        m_currentFramesPerSecond = static_cast<double>(1) / static_cast<double>(dt);

        // increment the frame counter
        m_frame++;
        
        // poll the window events. this calls any callbacks first
        // this should have actually been done earlier but oh well.
        // without this, inputs would be handled on the NEXT frame
        // instead of this one.
        preFramePollEvents();

        if (!cam) {
            Logging::Logger::get().logError("No active camera in game! There has to be an active camera for any behaviour to occurr.");
        }
        else {
            glClear(cam->m_clearFlags);
            glClearColor(cam->m_clearCol.x, cam->m_clearCol.y, cam->m_clearCol.z, cam->m_clearCol.w);
        }

        // poll all component behaviours
        Entity::_pollComponentBehaviours(dt);

        // cycle the glfw front and back buffers
        glfwSwapBuffers(m_wnd); 
        
        // end of frame, viewport no longer dirty
        // comes before glfwPollEvents because the 
        // framebuffer resize gets called there.
        m_viewportDirty = false;

        // moved Input::NewFrame() to postFramePollEvents 
        // (and renamed it to poll()) to clean up.
        // we might have other things that
        // have to go here so it's nicer to have them
        // all in one place.
        postFramePollEvents();
    }
    // we have left the loop, so the program is attempting to close.
    cleanup();
}

void Window::preFramePollEvents() {
    // poll glfw events.
    glfwPollEvents(); 
    ObjectRegistry::pollGC();
}

// poll the various program events
void Window::postFramePollEvents() {
    // update the input system
    Input::poll();
}

// clean up the program after exiting
void Window::cleanup() {
    Logging::Logger::get().logInfo("Program recieved shutdown request, or was shutdown by the user. Attempting to clean up resources before closing...");
    ResourceManager::freeArchives();
    GLContext::get().destroyGLContext();
    Logging::Logger::closeLog();
}