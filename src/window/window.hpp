#pragma once

#include "../gl/gl_context.hpp"
#include "../include/fm/fmath.hpp"
#include <string>


namespace Engine {

    // FullscreenMode is just an enumerator to store different window states.
    // pretty self explanitory.
    enum FullscreenMode {
        Fullscreen = 0x01,
        Borderless = 0x02,
        BorderlessWindowed = 0x03,
        Windowed = 0x04
    };

    // to keep the program simple, the window class is a singleton.
    // i made this design choice because it's easy to keep track of
    // and it's easy to just get the window and do stuff without
    // having to make sure you're getting the "main window".

    class Window {
    protected:

        // this is only used when swapping between fullscreen modes.
        // if in fullscreen, the old pos and size will be cached here
        // so the window position and size can be faithfully restored
        // if the program returns to windowed mode.
        fm::uvec2 m_wndSize_old;
        fm::uvec2 m_wndPos_old; 
        
        // window size. uvec2 because these values can never be negative or decimals.
        // (uvec is just a vector of type unsigned integer. equivalent to fm::_vec2<unsigned int>)
        fm::uvec2 m_wndSize = fm::uvec2(800,800);

        // window title
        std::string m_wndTitle = "Game Window";
        
        // GLFW window handle. required for a lot of things as you may know.
        // i believe on windows there is a function to get the HWND handle from this.
        GLFWwindow* m_wnd = nullptr;

        // store the current window state as well.
        FullscreenMode m_fsMode = FullscreenMode::Windowed;

        // this function defines what glfw window hints are set by default.
        // i.e glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // this is passed through to the GLContext on window creation as a lambda.
        static void _default_window_hints();

        // handles changing the m_wndSize value and glViewport.
        static void _framebuffer_resize_callback(GLFWwindow* wnd, int w, int h);

    public:

        // there's a lot of these dotted around the program for a good reason.
        // check out gl_context.hpp for an explanation of why.
        static Window& get() {
            static Window instance;
            return instance;
        }

        // returns the window size.
        fm::uvec2 getWndSize();

        // sets the window size via glfwSetWindowSize(m_wnd, v.x, v.y).
        void setWndSize(fm::uvec2 v);
        

        // returns the window title
        std::string getWndTitle();

        // sets the window title via glfwSetWindowTitle(m_wnd, s.c_str()).
        void setWndTitle(std::string s);


        // returns the glfw window handle. (different from the HWND window handle.)
        GLFWwindow* getWndHandle() const;

        // creates the window. this should be the first function to run.
        // a run through of what the parameters defaults are for:
        
        // size: the window size. leave at -1 for the default value of 800x800. 
        // title: window title. -1 for default also.
        // fsMode: fullscreen mode. by default is just windowed.
        // monitor: leave null for now.
        // share: i don't really know what this does, it just gets passed through to glfwCreateWindow.
        // extra_window_hints: a lambda function to add your own window hints in. create a function that
        // has glfwWindowHint() functions and pass that function through as a parameter. not required.
        int createWindow(fm::uvec2 size = fm::uvec2(-1), std::string title = "-1", FullscreenMode fsMode = FullscreenMode::Windowed, GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL, void(*extra_window_hints)() = nullptr);

        // returns the current window fullscreen state.
        FullscreenMode getFullscreenMode();

        // sets the window state via a switch statement. 
        void setFullscreenMode(FullscreenMode fm);

        // begins the program rendering and logic loop. in main(), this should come last unless you have
        // any cleanup stuff to perform after the program stops.
        void beginGameLoop();
    };
}