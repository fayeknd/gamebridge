#include "mesh.hpp"
#include "shader.hpp"

using namespace Engine;

// create a mesh with any valid mesh data.
// vertices and indices. and also topology.
std::shared_ptr<Mesh> Mesh::createMeshWithData(std::vector<Vertex>& vertices, std::vector<GLushort>& indices, GLuint topology) {

    // make sure the gl context exists first.
    if (!GLContext::get().isGLReady()) return nullptr;
    // create the new mesh
    auto mesh = std::shared_ptr<Mesh>(new Mesh());
    mesh->m_topology = topology;
    mesh->m_indexCount = indices.size(); 

    // create the buffers and array
    // with stupid opengl calls
    glGenBuffers(1, &mesh->m_vbo);
    glGenBuffers(1, &mesh->m_ebo);
    glGenVertexArrays(1, &mesh->m_vao);

    // bind the vertex array first.
    // the vertex array can be seen as the "parent"
    // for the vbo and ebo.
    glBindVertexArray(mesh->m_vao);

    // bind the vbo
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo);
    // copy the vertices to the vertex buffer.
    // vertices.size() * sizeof(Vertex) returns 
    // the required size (in bytes) the buffer needs to be.
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // bind the element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ebo);
    // same thing, copy the indices here.
    // after this, the vertices and indices vectors
    // are done and we don't need them anymore. 
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

    // set the vertex attributes (layout (location = x))

    // position
    glVertexAttribPointer(shaderVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(shaderVertexAttrib_Position);

    //colour
    glVertexAttribPointer(shaderVertexAttrib_Colour,   4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(shaderVertexAttrib_Colour);

    // uv/texture coordinate
    glVertexAttribPointer(shaderVertexAttrib_UV,       2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(shaderVertexAttrib_UV);

    // normal
    glVertexAttribPointer(shaderVertexAttrib_Normal,   3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(shaderVertexAttrib_Normal);

    // vao setup done, unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // mesh set up!
    return mesh;
}

// bind this as the active mesh to be drawn.
void Mesh::bindMesh() {
    // if the currently bound mesh is this, return early.
    if (GLContext::get().glState.m_currentMesh == m_vao) return;
    // else, bind the mesh. 
    glBindVertexArray(m_vao);
}

// unimplemented
std::shared_ptr<Mesh> Mesh::createPrimitiveMesh(std::string mesh) {
    return nullptr; // doesn't do anything atm
}

// deconstructor
Mesh::~Mesh() {

    // if the glcontext is not ready, the mesh
    // cannot have been created.
    if (!GLContext::get().isGLReady()) return;

    // unbind everything just in case.
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // delete buffers.
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}