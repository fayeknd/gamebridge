#include <component/renderable.hpp>
#include <entity.hpp>

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
    glFrontFace(rd.m_frontFace);
    glPolygonMode(rd.m_cullFace, rd.m_polyMode);
}

// draw
void IRenderable::lateUpdate(double delta) {
    // if the mesh or shader is null, or if the renderer is disabled, return early
    if (!m_mesh || !m_shaderProgram || !m_render) return;
    _useRenderData(); // use our render data

    // bind the mesh
    m_mesh->bindMesh();

    // use the program 
    m_shaderProgram->useProgram();

    // set basic uniforms
    m_shaderProgram->setUniform(shader_ModelMatrix, entity.transform.getMatrix());
    /* temp */ m_shaderProgram->setUniform(shader_ViewProjectionMatrix, fm::mat4(1));

    // draw the mesh
    glDrawElements(m_mesh->getTopology(), m_mesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);

}