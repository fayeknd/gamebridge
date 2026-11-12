#include <editorui/editorui.hpp>
#include <debug.hpp>

using namespace Engine;

void EditorUI::init() {

    m_uiCam = std::shared_ptr<Camera>(entity.addComponent<Camera>());
    m_io = Debug::DebugUI::get().getIO();

}