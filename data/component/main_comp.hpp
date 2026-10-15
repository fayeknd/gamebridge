#pragma once
#include <component/component.hpp>

namespace Engine {
    
    class MainComponent : public IComponent {

    public:

        void init() override;
        void update(float delta) override;
        void lateUpdate(float delta) override; 
        void onDestroy() override;
        
    };
}