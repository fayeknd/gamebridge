#include <iostream>

#include "../include/glad/glad.h"
#include "../include/GLFW/glfw3.h"
#include "error.h"


int main(int, char**){
    std::cout << "Hello !\n";

    glfwInit();
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);

    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    GLFWwindow* wnd = glfwCreateWindow(100, 100, "Window", NULL, NULL);
    if (!wnd) {
        std::cerr << "Window creation failed!\n";
        return ERR_WINDOW_CREATION_FAILED;
    }
    glfwMakeContextCurrent(wnd);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD OpenGL loading failed!\n";
        return ERR_GLAD_GL_LOAD_FAILED;
    }


    glClearColor(1.0f, 0.5f, 1.0f, 0.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(wnd)) {

        glClearColor(0,0,0,0);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(wnd); 
        glfwPollEvents();
    }
    glfwDestroyWindow(wnd);
    glfwTerminate();
    return 0;
}
