#pragma once

#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include "../engine.h"


namespace Engine {

    class GLContext {
    protected:

        bool m_glInit = false;

    public:

        static GLContext& get() {
            static GLContext instance;
            return instance;
        }

        bool& isGLReady();
        int createGLContext(void(*window_hints)(), void(*extra_window_hints)() = nullptr);
        int destroyGLContext();

        ~GLContext();

    };

}