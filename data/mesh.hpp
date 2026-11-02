#pragma once
#include <engine.h>
#include <gl_context.hpp>

#include <iostream>
#include <memory>
#include <vector>
#include <vertex.hpp>
#include <idpool.hpp>
#include <cachable.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <material.hpp>

namespace Engine {

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

        // create a mesh with any valid mesh data.
        // vertices and indices. and also topology.
        static std::shared_ptr<Mesh> createMeshWithData(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint topology = GL_TRIANGLES);

        // NOT artificial intelligence, assimp.
        // convert a mesh from the assimp mesh/scene pair to our mesh type
        static std::shared_ptr<Mesh> createAiMesh(aiMesh* mesh, const aiScene* scene, std::vector<std::shared_ptr<Material>>* materialList = nullptr);

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