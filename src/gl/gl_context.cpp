#include "gl_context.hpp"
#include "../window/window.hpp"
using namespace Engine;


// self-explanitory, returns true if the glcontext is usable and false if it is not usable.
bool& GLContext::isGLReady() {
    return m_glInit;
}

// creates the glfw context. will only create the gl context if m_glInit is false.
// window hints and extra window hints are lambda functions passed in and called inside the function.
// window_hints is the function for the default glfw window hints (i.e GLFW_TRANSPARENT_FRAMEBUFFER).
// these are set in window.hpp.

// extra_window_hints is an additional optional stage for setting any extra alternative window hints. not required.
int GLContext::createGLFWContext(void(*window_hints)(), void(*extra_window_hints)()) {

    // context has already been created, just return early
    if (isGLReady()) return ERR_CONTEXT_EXISTS;

    // glfwinit failed lol
    if (!glfwInit()) return ERR_GLFW_INIT_FAILED;

    // run the window_hints function (belonging to the window class)
    window_hints();
    // if the extra_window_hints function is not a nullptr, then run that too.
    if (extra_window_hints) extra_window_hints();

    // glInit is now true.
    m_glInit = true;
}
// this one is annoying. the GLAD context is actually responsible for creating the GL context and ideally would have
// been created with the createGLFWContext function, combining into the createGLContext function.
// unfortunately, the glad context HAS to be made after the main window.
int GLContext::createGLADContext() {

    // i genuinely don't know what this does or why it has to look like this.
    // like everyone, I just took this from learnopengl.com. (https://learnopengl.com/Getting-started/Hello-Window)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

        // if we made it into here, the glad loader failed.
        std::cerr << "GLAD OpenGL loading failed!\n";
        return ERR_GLAD_GL_LOAD_FAILED;
    }

    return GLAD_LOAD_SUCCESS;
}

// destroys the gl(fw) context. again, will only destroy the context if m_glInit is true.
// this is called by the destructor, so there isn't really a need to call this yourself
// unless you're doing something really specific. 
int GLContext::destroyGLContext() {

    // quick return if the context already doesn't exist
    if (!isGLReady()) return ERR_CONTEXT_NULL;

    // destroy the window first
    glfwDestroyWindow(Window::get().getWndHandle());
    
    // then terminate the glfw instance.
    glfwTerminate();

    // glInit is now false.
    m_glInit = false;
    return 0;
}

// destructor just calls destroyGLContext().
GLContext::~GLContext() {
    destroyGLContext();
}