#pragma once
#include <engine.h>
#include <gl_context.hpp>

#include <iostream>
#include <memory>
#include <vector>
#include "vertex.hpp"
#include <idpool.hpp>

namespace Engine {

    // just one currently
    enum Primitive {
        Cube     = 0x01
    };

    using MeshID = uint16_t;

    // the mesh class is responsible for holding all of the
    // annjoying verbose opengl boilerplate.

    class Mesh {
    private:

        // vertex buffer id
        GLuint m_vbo;

        // element/index buffer id
        GLuint m_ebo;

        // vertex array id
        GLuint m_vao;

        // this mesh's topology. used in glDrawElements.
        // this is GL_TRIANGLES by default, it's best
        // to leave it as it is if you don't know
        // what it's really for.
        GLuint m_topology = GL_TRIANGLES;

        // get the amount of indices in the mesh. 
        // useful for drawing with glDrawArrays
        // without having to store a separate 
        // buffer/vector of indices to get the
        // count from.
        GLuint m_indexCount;

        friend class Primitives;

    public:

        // deconstructor, just destroys the various buffers.
        ~Mesh();

        // create a primitive mesh using an identifier. 
        // CURRENTLY DOESN'T DO ANYTHING!
        static std::shared_ptr<Mesh> primitive(Primitive p);

        // create a mesh with any valid mesh data.
        // vertices and indices. and also topology.
        static std::shared_ptr<Mesh> createMeshWithData(std::vector<Vertex>& vertices, std::vector<GLushort>& indices, GLuint topology = GL_TRIANGLES);

        // bind this as the active mesh to be drawn.
        void bindMesh();

        // getters for the private variables.
        GLuint const getVBO() { return m_vbo; } 
        GLuint const getEBO() { return m_ebo; }
        GLuint const getVAO() { return m_vao; }
        GLuint const getTopology() { return m_topology; }
        GLuint const getIndexCount() { return m_indexCount; }

    };
}