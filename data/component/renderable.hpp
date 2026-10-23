#pragma once
#include <component/component.hpp>
#include <mesh.hpp>
#include <shader.hpp>
#include <gl_context.hpp>

namespace Engine {
    
    // a basic renderable interface.
    // will take a mesh and shader (to be material w/ shader)
    // and draws them.
    class IRenderable : public IComponent {
    protected:

    public:

        // instance's render data
        RenderData m_renderData = RenderData();

        // mesh to draw
        std::shared_ptr<Mesh> m_mesh;
        
        // shader to draw
        std::shared_ptr<ShaderProgram> m_shaderProgram;

        // marks if this instance should render or not
        bool m_render = true;

        // general setup
        void init() override;

        // where the draw happens
        void draw() override;
        
    };
}