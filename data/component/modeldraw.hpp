#pragma once
#include <component/component.hpp>
#include <model.hpp>
#include <shader.hpp>
#include <gl_context.hpp>
#include <material.hpp>

namespace Engine {
    
    // a basic renderable interface.
    // will take a mesh and shader (to be material w/ shader)
    // and draws them.
    class ModelDraw : public IComponent {
    protected:

    component_public:

        // instance's render data
        PUBLIC RenderData m_renderData = RenderData();

        // model to draw
        PUBLIC std::shared_ptr<Model> m_model = nullptr;
        
        // material to use
        //std::shared_ptr<Material> m_material = nullptr; moved to component.hpp (explanation there)

        // general setup
        void init() override;

        // where the draw happens
        void draw() override;
        
    };
}