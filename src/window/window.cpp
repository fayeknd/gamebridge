#include "window.hpp"

#include "../include/GLFW/glfw3native.h"
#include "../data/resources.hpp"
#include "../data/shader.hpp"
using namespace Engine;

GLFWwindow* Window::getWndHandle() const {
    return m_wnd;
}

fm::uvec2 Window::getWndSize() {
    return m_wndSize;
}
void Window::setWndSize(fm::uvec2 v) {
    m_wndSize = v;
    if (GLContext::get().isGLReady()) glfwSetWindowSize(m_wnd, v.x, v.y);
}

std::string Window::getWndTitle() {
    return m_wndTitle;
}
void Window::setWndTitle(std::string s) {
    m_wndTitle = s;
    if (GLContext::get().isGLReady()) glfwSetWindowTitle(m_wnd, s.c_str());
}

FullscreenMode Window::getFullscreenMode() {
    return m_fsMode;
}
void Window::setFullscreenMode(FullscreenMode fm) {
    if (fm == m_fsMode) return; 
    switch (fm) {
        case FullscreenMode::Fullscreen: {
            int w, h;
            
            glfwGetWindowSize(m_wnd, &w, &h);
            m_wndSize_old = fm::uvec2(w, h);
            int x, y;

            glfwGetWindowPos(m_wnd, &x, &y);
            m_wndPos_old = fm::uvec2(x, y);

            const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            glfwSetWindowMonitor(m_wnd, glfwGetPrimaryMonitor(), 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate); 
            break; }

        case FullscreenMode::Windowed: {
            
            if (m_fsMode == FullscreenMode::Fullscreen) {
                glfwSetWindowMonitor(m_wnd, NULL, m_wndPos_old.x, m_wndPos_old.y, m_wndSize_old.x, m_wndSize_old.y, 0);
                break;
            }
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_FALSE);
            break; } 

            // Don't know if this works on windows, should do
        case FullscreenMode::BorderlessWindowed: {

            glfwHideWindow(m_wnd);
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_TRUE);
            glfwPollEvents();
            glfwShowWindow(m_wnd);
            glfwFocusWindow(m_wnd);
            
            
            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_FALSE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; }

        case FullscreenMode::Borderless: {

            glfwHideWindow(m_wnd);
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_TRUE);
            glfwPollEvents();
            glfwShowWindow(m_wnd);
            glfwFocusWindow(m_wnd);


            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_TRUE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; } 
    }
    m_fsMode = fm;
}

#include "../data/mesh.hpp"
#include "../data/shader.hpp"

std::shared_ptr<Mesh> mesh;
std::shared_ptr<ShaderProgram> program;


int Window::createWindow(fm::uvec2 size, std::string title, FullscreenMode fsMode, GLFWmonitor *monitor, GLFWwindow *share, void(*extra_window_hints)()) {
    if (GLContext::get().isGLReady()) return ERR_CONTEXT_EXISTS;
    if (GLContext::get().createGLContext(_default_window_hints, extra_window_hints) != GL_CREATE_CONTEXT_SUCCESS)
        return ERR_WINDOW_CREATION_FAILED;

    if (m_wnd) return ERR_WINDOW_CREATION_FAILED;

    if (size == fm::uvec2(-1)) size = m_wndSize;
    if (title == "-1") title = m_wndTitle;
    
    m_wnd = glfwCreateWindow(size.x, size.y, title.c_str(), monitor, share);
    m_wndSize = size;
    m_wndTitle = title;
    
    if (!m_wnd) {
        return ERR_WINDOW_CREATION_FAILED;
    }

    glfwMakeContextCurrent(m_wnd);
    glfwSetFramebufferSizeCallback(m_wnd, _framebuffer_resize_callback);

    // would have been in GLContext::createGLContext(), but this has to run after window creation.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD OpenGL loading failed!\n";
        return ERR_GLAD_GL_LOAD_FAILED;
    }
    glViewport(0, 0, size.x, size.y);
    setFullscreenMode(fsMode);

    auto v = std::vector<Vertex>() = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0,1.0,1.0,0.0}, {0.0,0.0}, {0.0,0.0,0.0}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}},
        {{-0.5f, -0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}}
    };
    auto i = std::vector<unsigned short>() = {0, 1, 2};
    mesh = Mesh::createMeshWithData(v, i);


    auto file = ResourceManager::get().loadRaw("resources=>shaders/shader.glsl");
    
    if (file.size() > 0) {
        program = ShaderProgram::createProgramFromData(file.data(), "testshader");
        program->useProgram();
    }
    ResourceManager::get().unloadArchives();
    

    return WINDOW_CREATION_SUCCESS;
}

void Window::_default_window_hints() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    #ifndef _WIN32
    glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    #endif    

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
}

void Window::_framebuffer_resize_callback(GLFWwindow* wnd, int w, int h) {
    glViewport(0, 0, w, h);
    Window::get().m_wndSize = fm::vec<unsigned int>(w,h);
}

void Window::beginGameLoop() {
    while (!glfwWindowShouldClose(m_wnd)) {
        
        glClear(GL_COLOR_BUFFER_BIT);
        program->useProgram();
        mesh->bindMesh();
        glDrawElements(mesh->getTopology(), mesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);
        
        glfwSwapBuffers(m_wnd); 
        glfwPollEvents();
    }
}