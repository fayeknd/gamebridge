#pragma once
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <fm/fmath.hpp>
#include <shader.hpp>
#include <vertex.hpp>

namespace Engine{

    class Entity;
    using EntityID = uint32_t;

    namespace Debug {
        

        class DebugUI {
        private:

            bool m_hasInit = false;

            ImGuiIO* m_io;
            ImFont* m_font;
            ImFontConfig m_font_cfg;

            void _drawTreeRow(std::weak_ptr<Entity> e);
            void _drawProperties(std::weak_ptr<Entity> e);
            
            std::weak_ptr<Entity> m_selectedEntity;

        public:

            static DebugUI& get() {
                static DebugUI instance; 
                return instance;
            }

            bool m_drawDebugTools    = false;
            bool m_drawDebugger      = false;


            void drawImGuiInterface_framebegin();
            void drawImGuiInterface_frameend();
            void drawDebugger();
            void drawPropertyEditor();
            void _createImGuiContext(GLFWwindow* wnd);
            void _destroyImGuiContext();

            ImGuiIO& getIO() { return *m_io; }

        };
        
        class DebugLine {
        private:

            GLuint m_vao;
            GLuint m_vbo; 
            std::shared_ptr<ShaderProgram> m_program;

            void _genLine();
            void _deleteLine();

        public:

            RenderData m_renderData;

            DebugLine();
            ~DebugLine();
            void draw(std::vector<Vertex> points, fm::mat4 model = fm::mat4::identity());

        };
    }
}