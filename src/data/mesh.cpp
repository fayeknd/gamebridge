#include <mesh.hpp>
#include <shader.hpp>
#include <texture.hpp>

using namespace Engine;

// create a mesh with any valid mesh data.
// vertices and indices. and also topology.
std::shared_ptr<Mesh> Mesh::createMeshWithData(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint topology) {

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
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

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

std::shared_ptr<Mesh> Mesh::createAiMesh(aiMesh* mesh, const aiScene* scene, std::vector<std::shared_ptr<Material>>* materialList) {
    // make sure the gl context exists first.
    if (!GLContext::get().isGLReady()) return nullptr;
    // create the new mesh
    auto _mesh = std::shared_ptr<Mesh>(new Mesh());

    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;

    // check if this mesh has UVs / Vertex colours
    bool hasUVs = (mesh->mTextureCoords[0] != nullptr);
    bool hasColours = (mesh->mColors[0] != nullptr);
    // get the vertices
    for (size_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex v;
        // set the pos, colour, normal
        v.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };
        v.normal = {
            mesh->mNormals[i].x,
            mesh->mNormals[i].y,
            mesh->mNormals[i].z
        };
        if (hasColours) {
            v.colour = {
                mesh->mColors[0][i].r,
                mesh->mColors[0][i].g,
                mesh->mColors[0][i].b,
                mesh->mColors[0][i].a,
            };
        }
        // does this mesh have UVs?
        if (hasUVs) {
            // yes, set them
            v.uv = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };
        }
        // push back this vertex
        vertices.push_back(v);
    } 
    // get the indices
    for (size_t i = 0; i < mesh->mNumFaces; i++) {
        // get this face
        aiFace face = mesh->mFaces[i];
        // push back all three face incides
        for (size_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }
    // does the mesh have materials? (and do we have a material list passed in?)
    if(mesh->mMaterialIndex >= 0 && materialList != nullptr) {
        if (materialList->size() > 0) materialList->clear();
        // yes, try load them.
        aiMaterial* aiMaterial = scene->mMaterials[mesh->mMaterialIndex];

        // try get the textures
        auto albedo   = Texture2D::createTextureFromAi(aiMaterial, scene, TextureType::Albedo);
        auto normal   = Texture2D::createTextureFromAi(aiMaterial, scene, TextureType::Normal);
        auto metallic = Texture2D::createTextureFromAi(aiMaterial, scene, TextureType::Metallic);
        auto emission = Texture2D::createTextureFromAi(aiMaterial, scene, TextureType::Emission);

        // if any of these textures are valid
        if (albedo || normal || metallic || emission) {

            // create the material
            auto material = Material::create();
            material->setAlbedoTx(albedo);
            material->setNormalTx(normal);
            material->setMetallicTx(metallic);
            material->setEmissionTx(emission);

            material->m_data.m_hasTransparency = true;
            // push back our material
            materialList->push_back(material);
        }
    }  
    // create the mesh
    return Mesh::createMeshWithData(vertices, indices, GL_TRIANGLES);
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