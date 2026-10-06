#pragma once
#include "../engine.h"
#include "../gl/gl_context.hpp"

#include <iostream>
#include <memory>
#include <vector>
#include "vertex.hpp"

namespace Engine {
    class Mesh {
    private:

        // okay so a mesh class doesn't actually NEED to store the vertex and index data to render anything,
        // thats the job of the vertex buffer / vertex array / element buffer. 
        // if you NEED to know the specific vertices, you probably don't. if you REALLY need to know them,
        // store them separately for the mesh you need. The only time I could think of somebody needing 
        // the vertex data is if they were modifying the mesh, which is an expensive task anyway and can 
        // be done in a vertex shader.

        GLuint m_vbo;
        GLuint m_ebo;
        GLuint m_vao;
        GLuint m_topology = GL_TRIANGLES;

    public:

        ~Mesh();
        static std::shared_ptr<Mesh> createPrimitiveMesh(std::string mesh);
        static std::shared_ptr<Mesh> createMeshWithData(std::vector<Vertex>* vertices, std::vector<GLushort>* indices, GLuint topology = GL_TRIANGLES);

        void bindMesh();

        GLuint const getVBO() { return m_vbo; } 
        GLuint const getEBO() { return m_ebo; }
        GLuint const getVAO() { return m_vao; }

    };
}