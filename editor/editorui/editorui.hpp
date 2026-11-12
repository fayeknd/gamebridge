#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>
#include <imgui/imgui.h>

namespace Engine {

    class EditorUI : public IComponent {
    cpublic:

        // camera used for the ui
        std::shared_ptr<Camera> m_uiCam = nullptr;
        // shortcut to the imgui IO
        ImGuiIO* m_io;

        void init() override;
        void lateUpdate(double delta) override;

        void drawUI();

    private:
    
    };
}