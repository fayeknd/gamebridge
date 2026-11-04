#include <model.hpp>
#include <component/camera.hpp>
#include <primitives.hpp>

using namespace Engine;


// create a model with verted and index data
std::shared_ptr<Model> Model::createModelWithData(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint topology) {
    // make sure the gl context exists first.
    if (!GLContext::get().isGLReady()) return nullptr;
    // create the new MODEL!!!
    auto model = std::shared_ptr<Model>(new Model());
    // push back a new mesh, made with the passed-in data.
    model->m_meshes.push_back(Mesh::createMeshWithData(vertices, indices, topology));
    // register this model
    model->_register();
    return model; // return the new model!
}

std::shared_ptr<Model> Model::createModelWithMeshes(std::vector<std::shared_ptr<Mesh>> meshes) {
    // make sure the gl context exists first.
    if (!GLContext::get().isGLReady()) return nullptr;
    // create the new MODEL!!! (ctrl+c/ctrl+v)
    auto model = std::shared_ptr<Model>(new Model());
    // set the meshes
    model->m_meshes = meshes;
    // register this model
    model->_register();
    return model; // cool
}

std::shared_ptr<Model> Model::createModelFromPath(std::filesystem::path path, std::vector<std::shared_ptr<Material>>* materials, unsigned int flags) {

    // make sure the gl context exists first.
    if (!GLContext::get().isGLReady()) return nullptr;

    // create an importer instance
    Assimp::Importer importer;

    // try get the model from an archive
    bytes rcFile = ResourceManager::loadRaw(path.string()); 
    if (rcFile.size() <= 0) {
        // file was not in an archive.
        // verify the path exists
        std::error_code ec;    
        if (!std::filesystem::exists(path, ec)) {
            // file read failed, print error message.
            log().logError("Model '{}' file read error! -> {}", path.string(), ec.message());
            return nullptr;
        }
        std::ifstream file(path);
        // read file contents into a buffer
        std::stringstream buf;
        buf << file.rdbuf();
        
        // convert the stringstream to an unsigned buffer vector
        std::string bufstr = buf.str();
        rcFile = bytes(bufstr.begin(), bufstr.end());
        
        // close file. data is in the buffer now.
        file.close();
    }
    // otherwise, file was in archive. either way rcFile contains
    // the bytes of the model now.

    // load the asssimp scene
    const aiScene* scene = importer.ReadFileFromMemory(
        rcFile.data(),
        rcFile.size(),
        flags,
        nullptr
    );

    // verity assimp scene is valid
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        log().logError("Assimp Error '{}' on Model '{}'.", importer.GetErrorString(), path.string());
        return nullptr;
    }

    // make the new model
    auto model = std::shared_ptr<Model>(new Model);
    // process the submeshes
    model->_processNode(scene->mRootNode, scene, materials);
    return model;
}

void Model::_processNode(aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Material>>* materials) {
    // loop through all the meshes in the node
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        // this is whacky. i love you joey de fries 
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(Mesh::createAiMesh(mesh, scene, materials));
    }
    // iterate down the hierarchy
    for (size_t i = 0; i < node->mNumChildren; i++) {
        _processNode(node->mChildren[i], scene, materials);
    }
}

void Model::_drawImpl(const std::vector<std::shared_ptr<Material>>& materials, fm::mat4 model, fm::mat4 vp) {
    
    // dont know when these would be LESS than zero, but whatev
    if (materials.size() <= 0 || m_meshes.size() <= 0) return;
    // mandatory null check
    if (!materials[0] || !m_meshes[0]) return; 

    // loop through each mesh, bind it and draw it.
    for (size_t i = 0; i < m_meshes.size(); i++) {
        // if the materials vector is smaller, then when the mesh index exceeds
        // the material index, we just use the last material in the vector.
        // given we have ONE material, and three meshes, the last valid
        // material (at index 0) will be used for all meshes.
        if (materials.size() >= (i + 1)) {
            
            // use the material
            materials[i]->useMaterial();
            // set basic uniforms
            materials[i]->m_program->setUniform(shader_ModelMatrix, model);
            materials[i]->m_program->setUniform(shader_ViewProjectionMatrix, vp);

        }
        
        // draw the mesh itself
        m_meshes[i]->bindMesh();
        glDrawElements(m_meshes[i]->getTopology(), m_meshes[i]->getIndexCount(), GL_UNSIGNED_INT, 0);
    }
}

void Model::draw(const std::shared_ptr<Material>& material, fm::mat4 model, fm::mat4 vp) {
    _drawImpl({material}, model, vp); // pass this to the other draw function
}
void Model::draw(const std::vector<std::shared_ptr<Material>>& materials, fm::mat4 model, fm::mat4 vp) {
    _drawImpl(materials, model, vp);
}

std::shared_ptr<Model> Model::primitive(Primitive p) {
    return Primitives::getPrimitive(p);
}