#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <fm/fmath.hpp>

namespace Engine{

    namespace Debug {
        
        class DebugUI {
        private:

            bool m_hasInit = false;

            ImGuiIO* m_io;
            ImFont* m_font;
            ImFontConfig m_font_cfg;

        public:

            static DebugUI& get() {
                static DebugUI instance; 
                return instance;
            }

            bool m_drawDebugger = false;
            bool m_drawHitboxes = false;


            void drawImGuiInterface_framebegin();
            void drawImGuiInterface_frameend();
            void drawDebugger();
            void _createImGuiContext(GLFWwindow* wnd);
            void _destroyImGuiContext();

            ImGuiIO& getIO() { return *m_io; }

        };
    }
}