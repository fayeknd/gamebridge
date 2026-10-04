#include "gl_context.hpp"
#include "../window/window.hpp"
using namespace Engine;

bool& GLContext::isGLReady() {
    return m_glInit;
}

int GLContext::createGLContext(void(*window_hints)(), void(*extra_window_hints)()) {
    if (isGLReady()) return ERR_CONTEXT_EXISTS;
    if (!glfwInit()) return ERR_GLFW_INIT_FAILED;

    window_hints();
    if (extra_window_hints) extra_window_hints();

    m_glInit = true;
    return GL_CREATE_CONTEXT_SUCCESS;
}

int GLContext::destroyGLContext() {

    if (!isGLReady()) return ERR_CONTEXT_NULL;

    glfwDestroyWindow(Window::get().getWndHandle());
    glfwTerminate();
    return 0;
}

GLContext::~GLContext() {
    destroyGLContext();
}