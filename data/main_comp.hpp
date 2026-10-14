#pragma once
#include <component.hpp>

namespace Engine {
    
    class MainComponent : public EntityComponent {

    public:

        void init() override;
        void update(float delta) override;
        void lateUpdate(float delta) override; 
        void onDestroy() override;
        
    };
}