#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>

namespace Engine {
    class DebugCam : public IComponent {

    component_public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;
        void draw() override;
        void movementInput(double delta);
        void mouseInput(double delta);

        PUBLIC float cameraSpeed = 3.f;
        PUBLIC float sensitivity = 0.25;
        Camera* cam;
    };
}