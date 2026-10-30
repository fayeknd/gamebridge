#pragma once
#include <component/component.hpp>
#include <texture.hpp>
#include <debug.hpp>

namespace Engine {
    
    // The main entrypoint component. The program
    // will assign this to the root component automatically.
    class MainComponent : public IComponent {
        
    component_public:

        void init() override;
        void update(double delta) override;
        void lateUpdate(double delta) override;
        void physicsUpdate(double delta) override; 
        void onDestroy() override;
        void draw() override;

        Entity* square;
        Entity* cam;
        PUBLIC float sensitivity = 0.25;
        PUBLIC float speed = 3.0f;

    };
}