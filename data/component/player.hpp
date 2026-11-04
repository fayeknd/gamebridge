#pragma once
#include <component/component.hpp>
#include <component/camera.hpp>
#include <physics/rigidbody.hpp>
#include <component/collider.hpp>
#include <texture.hpp>
#include <debug.hpp>

namespace Engine {
    
    // The main entrypoint component. The program
    // will assign this to the root component automatically.
    class Player : public IComponent {
        
    component_public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;
        void draw() override;
        void movementInput(double delta);
        void mouseInput(double delta);

        Rigidbody* rb;
        Collider* collider;
        Entity* camObject;
        Camera* cam;
        PUBLIC float sensitivity = 0.25;
        PUBLIC float speed = 3.0f;
        PUBLIC bool toggleMovement;
        PUBLIC bool toggleMouseMovement;
    };
}