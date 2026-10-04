#include <iostream>
#include "window/window.hpp"

using namespace Engine;

int main(int, char** args){
    Window::get().createWindow({300,300}, args[1] ? args[1] : "-1");
    
    glClearColor(1.0f, 0.5f, 1.0f, 1.0f);
    
    
    Window::get().beginGameLoop();
}
