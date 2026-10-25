#include <gl_context.hpp>

using namespace Engine;

// use the render data
void RenderData::useRenderData() {
    glFrontFace(m_frontFace);
    glCullFace(m_cullFace);
    glPolygonMode(m_setPoly, m_polyMode);
    glBlendFunc(m_blendFuncOpt, m_blendFuncVal);
    glLineWidth(m_lineWidth);
}