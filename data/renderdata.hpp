#include <engine.h>
#include <GLFW/glfw3.h>

namespace Engine {

    // a simple struct containing some object render data.
    struct RenderData {

        // which direction defines "front" faces.
        // GL_CW for clockwise face winding.
        // GL_CCW for ant-clockwise face winding.
        unsigned int m_frontFace = GL_CW;

        // which face direction to cull.
        unsigned int m_cullFace = GL_BACK;

        // what polygon mode opengl is drawing in.
        // GL_FILL for regular full tri drawing,
        // GL_LINE for wireframe rendering.
        unsigned int m_polyMode = GL_FILL;
    };

    // comparator
    inline bool operator==(const RenderData& v1, const RenderData& v2) {
        return (
            v1.m_frontFace  == v2.m_frontFace  &&
            v1.m_cullFace   == v2.m_cullFace   &&
            v1.m_polyMode   == v2.m_polyMode
        );
    }
}