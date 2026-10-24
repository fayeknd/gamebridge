#pragma once 
#include <texture.hpp>
#include <shader.hpp>

namespace Engine {
    
    constexpr const char* materialUniform_albedoFac     = ".albedo_fac";
    constexpr const char* materialUniform_emissionFac   = ".emission_fac";
    constexpr const char* materialUniform_roughnessFac  = ".roughness_fac";
    constexpr const char* materialUniform_metallicFac   = ".metallic_fac";
    constexpr const char* materialUniform_ambientFac    = ".metallic_fac";

    constexpr const char* materialUniform_hasAlbedo     = ".has_albedo_tx";
    constexpr const char* materialUniform_hasNormal     = ".has_normal_tx";
    constexpr const char* materialUniform_hasMetallic   = ".has_metallic_tx";
    constexpr const char* materialUniform_hasEmission   = ".has_emission_tx";

    constexpr const char* materialUniform_albedoTx      = "u_albedo_tx";
    constexpr const char* materialUniform_normalTx      = "u_normal_tx";
    constexpr const char* materialUniform_metallicTx    = "u_metallic_tx";
    constexpr const char* materialUniform_emissionTx    = "u_emission_tx";

    // the physically-based rendering data.
    struct PBRData {
        fm::vec4 m_albedoFac                            = {1.0f, 1.0f, 1.0f, 1.0f};
        fm::vec4 m_emissionFac                          = {0.0f, 0.0f, 0.0f, 0.0f};
        float m_roughnessFac                            = 0.5f;
        float m_metallicFac                             = 0.0f;
        float m_ambientFac                              = 1.0f;
        
        bool m_hasAlbedoTx                              = false;
        bool m_hasNormalTx                              = false;
        bool m_hasMetallicTx                            = false;
        bool m_hasEmissionTx                            = false;

    };

    class Material : ICachable<Material> {
    protected:

        // texture pointers
        std::shared_ptr<Texture2D> m_albedoTx;
        std::shared_ptr<Texture2D> m_normalTx;
        std::shared_ptr<Texture2D> m_metallicTx;
        std::shared_ptr<Texture2D> m_emissionTx;


    public:

        // this will run the _genMaterial() function.
        // also return a pointer to this instance.
        static std::shared_ptr<Material> create();

        // texture setters/getters
        const std::shared_ptr<Texture2D> getAlbedoTx() const { return m_albedoTx; };
        const std::shared_ptr<Texture2D> getNormalTx() const { return m_normalTx; };
        const std::shared_ptr<Texture2D> getMetallicTx() const { return m_metallicTx; };
        const std::shared_ptr<Texture2D> getEmissionTx() const { return m_emissionTx; };
  
        void setAlbedoTx(std::shared_ptr<Texture2D> tx);
        void setNormalTx(std::shared_ptr<Texture2D> tx);
        void setMetallicTx(std::shared_ptr<Texture2D> tx);
        void setEmissionTx(std::shared_ptr<Texture2D> tx);

        // PBR data that stores PBR values.
        // is exactly 48 bytes. (or should be.)
        // uhh if you wanna add any thing to this,
        // DON'T!! lmk and i'll do it cause
        // ssbo's are fragile 
        PBRData m_data;
        
        // the shader program
        std::shared_ptr<ShaderProgram> m_program;

        // use this material for drawing.
        // (bind the textures.)
        void useMaterial();

    };
}