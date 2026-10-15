#include <component/renderable.hpp>
#include <entity.hpp>

using namespace Engine;

void IRenderable::init() {
}

void IRenderable::_useRenderData() {
    RenderData& rd = GLContext::get().glState.renderData;
    if (rd == m_renderData) return;
    m_renderDataBk = rd;
    rd = m_renderData;

    glFrontFace(rd.m_frontFace);
    glPolygonMode(rd.m_cullFace, rd.m_polyMode);
}

void IRenderable::lateUpdate(float delta) {
    if (!m_mesh || !m_shaderProgram || !m_render) return;
    //_useRenderData();

    m_mesh->bindMesh();
    m_shaderProgram->useProgram();
    m_shaderProgram->setUniform(shader_ModelMatrix, entity.transform.getMatrix());
    /* temp */ m_shaderProgram->setUniform(shader_ViewProjectionMatrix, fm::mat4(1));

    glDrawElements(m_mesh->getTopology(), m_mesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);

}