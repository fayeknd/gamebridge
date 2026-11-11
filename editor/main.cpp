#include <iostream>
#include <renderer.hpp>
#include <generated/generated_reflection.hpp>

using namespace Engine;

// don't add stuff here, add stuff in component/main_comp.cpp in the init() function.
int main(int, char** args){
    // this HAS to get called first so the compiler generates it first.
    Engine::initializeReflectionData();
    Renderer::get().createWnd({300,300}, args[1] ? args[1] : "-1");
    Renderer::get().beginGameLoop();
}
