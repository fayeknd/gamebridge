#include "mesh.hpp"
#include "shader.hpp"

using namespace Engine;

std::shared_ptr<Mesh> Mesh::createMeshWithData(std::vector<Vertex>* vertices, std::vector<GLushort>* indices, GLuint topology) {

    // it genuinely took me 2 years to realise that meshes did not need to store their vertices/indices as a class member

    auto mesh = std::shared_ptr<Mesh>(new Mesh());

    glGenBuffers(1, &mesh->m_vbo);
    glGenBuffers(1, &mesh->m_ebo);
    glGenVertexArrays(1, &mesh->m_vao);

    glBindVertexArray(mesh->m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices->size() * VERTEX_SIZE, vertices->data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices->size() * sizeof(GLushort), indices->data(), GL_STATIC_DRAW);

    glVertexAttribPointer(shaderVertexAttrib_Position, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid*)offsetof(Vertex, position));
    glEnableVertexAttribArray(shaderVertexAttrib_Position);

    glVertexAttribPointer(shaderVertexAttrib_Colour,   4, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(shaderVertexAttrib_Colour);

    glVertexAttribPointer(shaderVertexAttrib_UV,       2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(shaderVertexAttrib_UV);

    glVertexAttribPointer(shaderVertexAttrib_Normal,   3, GL_FLOAT, GL_FALSE, VERTEX_SIZE, (const GLvoid*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(shaderVertexAttrib_Normal);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    mesh->m_topology = topology;
    mesh->m_indexCount = indices->size(); 

    return mesh;
}

std::shared_ptr<Mesh> Mesh::createPrimitiveMesh(std::string mesh) {
    return nullptr; // doesn't do anything atm
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    glDeleteVertexArrays(1, &m_vao);
}