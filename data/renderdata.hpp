#include <engine.h>
#include <GLFW/glfw3.h>

namespace Engine {
    struct RenderData {
        unsigned int m_frontFace = GL_CW;
        unsigned int m_cullFace = GL_BACK;
        unsigned int m_polyMode = GL_FILL;
    };
    inline bool operator==(const RenderData& v1, const RenderData& v2) {
        return (
            v1.m_frontFace  == v2.m_frontFace  &&
            v1.m_cullFace   == v2.m_cullFace   &&
            v1.m_polyMode   == v2.m_polyMode
        );
    }
}