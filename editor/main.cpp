#include <iostream>
#include <renderer.hpp>
#include <generated/generated_reflection.hpp>
#include <entity.hpp>
#include <editorui/editorui.hpp>

using namespace Engine;

// don't add stuff here, add stuff in component/main_comp.cpp in the init() function.
int main(int, char** args){
    // this HAS to get called first so the compiler generates it first.
    Engine::initializeReflectionData();
    Renderer::get().createWnd({300,300}, args[1] ? args[1] : "-1");

    // add the main entrypoint component to the root entity.
    //Entity::s_root->addComponent<MainComponent>();
    Entity::s_root->addComponent<EditorUI>();

    Renderer::get().beginGameLoop();
}
