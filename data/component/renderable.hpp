#pragma once
#include <component/component.hpp>
#include <mesh.hpp>
#include <shader.hpp>
#include <gl_context.hpp>

namespace Engine {
    
    class IRenderable : public IComponent {
    protected:
    
        void _useRenderData();
        RenderData m_renderDataBk;

    public:

        RenderData m_renderData;

        std::shared_ptr<Mesh> m_mesh;
        std::shared_ptr<ShaderProgram> m_shaderProgram;

        bool m_render = true;

        void init() override;
        void lateUpdate(float delta) override;
        
    };
}