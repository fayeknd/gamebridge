#include <GL/gl.h>
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

        unsigned int m_setPoly = GL_FRONT_AND_BACK;

        // what polygon mode opengl is drawing in.
        // GL_FILL for regular full tri drawing,
        // GL_LINE for wireframe rendering.
        unsigned int m_polyMode = GL_FILL;
        
        unsigned int m_blendFuncOpt = GL_SRC_ALPHA;
        unsigned int m_blendFuncVal = GL_ONE_MINUS_SRC_ALPHA;
        
        // apply the render data
        void useRenderData();
    };

    // comparator
    inline bool operator==(const RenderData& v1, const RenderData& v2) {
        return (
            v1.m_frontFace    == v2.m_frontFace    &&
            v1.m_cullFace     == v2.m_cullFace     &&
            v1.m_setPoly      == v2.m_setPoly      &&
            v1.m_polyMode     == v2.m_polyMode     &&
            v1.m_blendFuncOpt == v2.m_blendFuncOpt &&
            v1.m_blendFuncVal == v2.m_blendFuncVal
        );
    }
}