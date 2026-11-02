#pragma once
#include <mesh.hpp>
#include <material.hpp>
#include <filesystem>

namespace Engine {

    // the primitive models
    enum Primitive {
        Cube     = 0x01,
        Plane    = 0x02
    };

    using ModelID = uint16_t;

    class Model : public ICachable<Model> {
    private:

        void _drawImpl(const std::vector<std::shared_ptr<Material>>& materials, fm::mat4 model, fm::mat4 vp);

        void _processNode(aiNode* node, const aiScene* scene, std::vector<std::shared_ptr<Material>>* materials);

        // logger
        static Logging::Logger& log() {
            static Logging::Logger instance("MODEL");
            return instance;
        }

    public:

        
        std::vector<std::shared_ptr<Mesh>> m_meshes;

        const size_t getMeshCount() const { return m_meshes.size(); }

        // create a primitive model using an identifier. 
        static std::shared_ptr<Model> primitive(Primitive p);

        // create a model with a single submesh made with the data.
        static std::shared_ptr<Model> createModelWithData(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, GLuint topology = GL_TRIANGLES);

        // create a model with a bunch of meshes
        static std::shared_ptr<Model> createModelWithMeshes(std::vector<std::shared_ptr<Mesh>> meshes);

        // create a model from a path or archive
        static std::shared_ptr<Model> createModelFromPath(std::filesystem::path path, std::vector<std::shared_ptr<Material>>* materials = nullptr, unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipWindingOrder);

        // draw the model (multi-materials)
        void draw(const std::vector<std::shared_ptr<Material>>& materials, fm::mat4 model, fm::mat4 vp);

        // draw the model (single-material)
        void draw(const std::shared_ptr<Material>& material, fm::mat4 model, fm::mat4 vp);

    };
}