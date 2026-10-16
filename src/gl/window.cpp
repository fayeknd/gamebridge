#include "window.hpp"

#include <GLFW/glfw3native.h>
#include <resources.hpp>
#include <shader.hpp>
#include <entity.hpp>
#include <component/main_comp.hpp>

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

    // if GLAD initialisation fails, return.
    if (GLContext::get().createGLADContext() != GLAD_LOAD_SUCCESS) {
        return ERR_WINDOW_CREATION_FAILED;
    }
    
    // ensure the glViewport is set to the window size properly.
    glViewport(0, 0, size.x, size.y);

    // set fullscreen mode passed in
    setFullscreenMode(fsMode);

    if (!Entity::s_root) {
        return ERR_NO_ROOT_ENTITY;
    }
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

    // make sure m_wndSize is aware of the updated size
    Window::get().m_wndSize = fm::vec<unsigned int>(w,h);
}

// begins the program rendering and logic loop. in main(), this should come last unless you have
// any cleanup stuff to perform after the program stops.
void Window::beginGameLoop() {

    // while glfw believes the window should not close
    while (!glfwWindowShouldClose(m_wnd)) {
        
        // TODO : move m_clearFlags to a future camera class
        glClear(GLContext::get().glState.m_clearFlags);

        // update all the components of all the entities 
        for (const auto& e : Entity::s_entityList) {
            e.second->_updateComponents(0);
        }
        for (const auto& e : Entity::s_entityList) {
            e.second->_lateUpdateComponents(0);
        }

        // cycle the glfw front and back buffers
        glfwSwapBuffers(m_wnd); 
        
        // update window.
        glfwPollEvents();
    }
    // we have left the loop, so the program is attempting to close.
    cleanup();
}

// clean up the program after exiting
void Window::cleanup() {
    ResourceManager::freeArchives();
    GLContext::get().destroyGLContext();
}