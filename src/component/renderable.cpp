#include <logger.hpp>
#include <shader.hpp>
#include <component/renderable.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <renderer.hpp>

using namespace Engine;

void IRenderable::init() {

    // initialise the material so we don't have to do this manually.
    // if we replace this, then the old reference will automatically
    // get discarded.
    m_materials.push_back(Material::create());
    entity.m_isRenderable = true;
}


// draw
void IRenderable::draw() {
    // if the model or material or shader is null, or if the renderer is disabled, return early
    if (!m_model) return;
    m_renderData.useRenderData(); // use our render data
    
    // draw the model
    m_model->draw(m_materials, 
        entity.getRealMatrix(),
        (Camera::activeCamera()) ? Camera::activeCamera()->getVPMatrix() : fm::mat4::identity()
    );

}