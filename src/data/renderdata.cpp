#include <gl_context.hpp>

using namespace Engine;

// use the render data
void RenderData::useRenderData() {
    glFrontFace(m_frontFace);
    glPolygonMode(m_cullFace, m_polyMode);
}