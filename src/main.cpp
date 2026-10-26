#include <iostream>
#include <renderer.hpp>

using namespace Engine;

// don't add stuff here, add stuff in component/main_comp.cpp in the init() function.
int main(int, char** args){
    Renderer::get().createWnd({300,300}, args[1] ? args[1] : "-1");
    Renderer::get().beginGameLoop();
}
