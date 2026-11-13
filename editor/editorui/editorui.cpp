#include <editorui/editorui.hpp>
#include <debug.hpp>
#include <entity.hpp>
#include <component/modeldraw.hpp>
#include <renderer.hpp>
#include <imgui/imgui_internal.h>
#include <debug.hpp>

using namespace Engine;

void EditorUI::init() {

    // make the ui cam
    m_uiCam = Entity::create("__EditorCam")->addComponent<Camera>();
    m_uiCam->m_clearCol = fm::col(0.1f, 0.1f, 0.1f, 1.0f);
    
    // get a ptr to the imgui io (shortcut really)
    m_io = &Debug::DebugUI::get().getIO();

    // TESTING
    auto cube = Entity::create("Cube")->addComponent<ModelDraw>();
    cube->m_model = Model::primitive(Primitive::Cube);
    cube->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));

    m_uiCam->getEntity().transform.addPosition({0, 1, -3});
    m_uiCam->getEntity().transform.addRotationX(10);

    Renderer::get().setWndSize({1920, 1080});
    Renderer::get().m_drawView = false; 

    m_uiCam->m_framebuffer->m_postProcessSteps.push_back(ShaderProgram::createProgramFromPath("stuff=>shaders/postprocess.glsl"));
    m_uiCam->m_framebuffer->setResizeWithWnd(false);
    
}

void EditorUI::lateUpdate(double delta) {

    ImGuiWindowFlags flags = {
          ImGuiWindowFlags_NoMove
        | ImGuiWindowFlags_NoResize
        | ImGuiWindowFlags_NoTitleBar
        | ImGuiWindowFlags_NoScrollbar
        | ImGuiWindowFlags_AlwaysAutoResize
        | ImGuiWindowFlags_NoScrollWithMouse
        | ImGuiWindowFlags_NoCollapse
    };

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    auto dockerID = ImGui::GetID("EditorDockspace");

    if (!ImGui::DockBuilderGetNode(dockerID)) {
        // clear out any existing layout states for this ID
        ImGui::DockBuilderRemoveNode(dockerID); 
        ImGui::DockBuilderAddNode(dockerID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockerID, ImGui::GetWindowSize());

        ImGuiID dockIDMain = dockerID;
        ImGuiID dockIDLeft;

        ImGui::DockBuilderSplitNode(dockIDMain, ImGuiDir_Left, 0.2f, &dockIDLeft, &dockIDMain);

        // bind our specific window titles to these localized layout slots
        ImGui::DockBuilderDockWindow("WorldViewport", dockIDMain);
        ImGui::DockBuilderDockWindow("Tools", dockIDLeft);

        ImGui::DockBuilderFinish(dockerID);
    }

    if (ImGui::DockSpaceOverViewport(dockerID, viewport)) {

        drawViewport(flags); // "Viewport"
        Debug::DebugUI::get().drawPropertyEditor(); // "Tools"

    }
    // we bound textures via ID this frame (imgui did) 
    // so fix the texture unit state
    Texture2D::m_unitStateBroken = true;

}

void EditorUI::drawViewport(ImGuiWindowFlags flags) {

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("WorldViewport"), NULL, ImGuiWindowFlags_NoCollapse) {
        // get the window viewport available size
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        // guard against collapsed windows
        if (viewportPanelSize.x > 0.0f && viewportPanelSize.y > 0.0f) {
            fm::uvec2 targetSize = {
                static_cast<uint32_t>(viewportPanelSize.x),
                static_cast<uint32_t>(viewportPanelSize.y)
            };

            // set the framebuffer size
            if (m_uiCam->m_framebuffer->setSize(targetSize)) {
                Renderer::get().m_viewportDirty = true;
            }

            // draw the camera view
            ImGui::Image(
                (ImTextureID)(intptr_t)m_uiCam->m_framebuffer->getRenderTexture()->getTextureID(),
                viewportPanelSize,
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();

}