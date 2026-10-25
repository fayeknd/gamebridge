#include <logger.hpp>
#include <shader.hpp>
#include <component/renderable.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <window.hpp>

using namespace Engine;

void IRenderable::init() {

    // initialise the material so we don't have to do this manually.
    // if we replace this, then the old reference will automatically
    // get discarded.
    m_material = Material::create();
}


// draw
void IRenderable::draw() {
    // if the mesh or material or shader is null, or if the renderer is disabled, return early
    if (!m_mesh || !m_material || !m_material->m_program || !m_render) return;
    m_renderData.useRenderData(); // use our render data

    // bind the mesh
    m_mesh->bindMesh();
    // use the material
    m_material->useMaterial();
    // set basic uniforms
    m_material->m_program->setUniform(shader_ModelMatrix, entity.transform.getGlobalMatrix());
    // ViewProjection matrix
    fm::mat4 vpMat = fm::mat4::identity();
    // if main camera exists
    if (Camera::activeCamera()) {
        // vpMat is the main camera projection * view
        vpMat = Camera::activeCamera()->getProjectionMat() * Camera::activeCamera()->getViewMat();
        // for debug
        /*
        std::cout << 
            vpMat.at(0, 0) << ", " << vpMat.at(1, 0) << ", " << vpMat.at(2, 0) << ", " << vpMat.at(3, 0) << ", " << std::endl << 
            vpMat.at(0, 1) << ", " << vpMat.at(1, 1) << ", " << vpMat.at(2, 1) << ", " << vpMat.at(3, 1) << ", " << std::endl << 
            vpMat.at(0, 2) << ", " << vpMat.at(1, 2) << ", " << vpMat.at(2, 2) << ", " << vpMat.at(3, 2) << ", " << std::endl << 
            vpMat.at(0, 3) << ", " << vpMat.at(1, 3) << ", " << vpMat.at(2, 3) << ", " << vpMat.at(3, 3) << std::endl;
            */
    }
    m_material->m_program->setUniform(shader_ViewProjectionMatrix, vpMat);
    // draw the mesh
    glDrawElements(m_mesh->getTopology(), m_mesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);

}