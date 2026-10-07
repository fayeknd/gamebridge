#pragma once

#include "../gl/gl_context.hpp"
#include "../include/fm/fmath.hpp"
#include <string>


namespace Engine {

    enum FullscreenMode {
        Fullscreen = 0x01,
        Borderless = 0x02,
        BorderlessWindowed = 0x03,
        Windowed = 0x04
    };

    class Window {
    protected:

        fm::uvec2 m_wndSize_old;
        fm::uvec2 m_wndPos_old; 
        
        fm::uvec2 m_wndSize = fm::uvec2(800,800);
        std::string m_wndTitle = "Game Window";
        GLFWwindow* m_wnd = nullptr;
        FullscreenMode m_fsMode = FullscreenMode::Windowed;
        static void _default_window_hints();
        static void _framebuffer_resize_callback(GLFWwindow* wnd, int w, int h);

    public:

        static Window& get() {
            static Window instance;
            return instance;
        }

        fm::uvec2 getWndSize();
        void setWndSize(fm::uvec2 v);
        
        std::string getWndTitle();
        void setWndTitle(std::string s);

        GLFWwindow* getWndHandle() const;
        int createWindow(fm::uvec2 size = fm::uvec2(-1), std::string title = "-1", FullscreenMode fsMode = FullscreenMode::Windowed, GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL, void(*extra_window_hints)() = nullptr);

        FullscreenMode getFullscreenMode();
        void setFullscreenMode(FullscreenMode fm);

        void beginGameLoop();
    };
}