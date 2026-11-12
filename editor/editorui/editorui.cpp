#include <editorui/editorui.hpp>
#include <debug.hpp>
#include <entity.hpp>
#include <component/modeldraw.hpp>
#include <renderer.hpp>

using namespace Engine;

void EditorUI::init() {

    // make the ui cam
    m_uiCam = Entity::create("__EditorCam")->addComponent<Camera>();
    m_uiCam->m_clearCol = fm::col(0.1f, 0.1f, 0.1f, 1.0f);
    
    // get a ptr to the imgui io (shortcut really)
    m_io = &Debug::DebugUI::get().getIO();

    // TESTING
    Entity::create("Cube")->addComponent<ModelDraw>()->m_model = Model::primitive(Primitive::Cube);
    
    m_uiCam->getEntity().transform.addPosition({0, -1, -3});
    m_uiCam->getEntity().transform.addRotationX(10);

    Renderer::get().setWndSize({1920, 1080});

}

void EditorUI::lateUpdate(double delta) {

    ImGui::Begin("TestWindow");
    ImGui::End();

}