#include <iostream>
#include <window.hpp>

using namespace Engine;

int main(int, char** args){
    Window::get().createWindow({300,300}, args[1] ? args[1] : "-1");
    Window::get().beginGameLoop();
}
