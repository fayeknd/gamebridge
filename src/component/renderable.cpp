#include <component/renderable.hpp>
#include <entity.hpp>
#include <component/camera.hpp>

using namespace Engine;

void IRenderable::init() {
}

// use this instance's render data
void IRenderable::_useRenderData() {

    // reference to the glstate data
    RenderData& rd = GLContext::get().glState.renderData;

    // if the render data is already the same, don't bother changing anything.
    if (rd == m_renderData) return;

    // backup the old render data
    m_renderDataBk = rd;
    // set the glstate render data to this render data
    rd = m_renderData;

    // apply the render data
    rd.useRenderData();
}

// draw
void IRenderable::draw() {
    // if the mesh or shader is null, or if the renderer is disabled, return early
    if (!m_mesh || !m_shaderProgram || !m_render) return;
    _useRenderData(); // use our render data

    // bind the mesh
    m_mesh->bindMesh();

    // use the program 
    m_shaderProgram->useProgram();

    // set basic uniforms
    m_shaderProgram->setUniform(shader_ModelMatrix, entity.transform.getGlobalMatrix());
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
    m_shaderProgram->setUniform(shader_ViewProjectionMatrix, vpMat);

    // draw the mesh
    glDrawElements(m_mesh->getTopology(), m_mesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);

}