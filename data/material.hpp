#pragma once 
#include <texture.hpp>
#include <shader.hpp>

namespace Engine {

    // love me type alias
    using bitmask = uint32_t;

    // a bitmask that stores flags for textures.
    // this is achieved with the bitwise operator
    // and internally looks like: 1111
    // |1| -> hasAlbedo is true
    // |0| -> hasNormal is false
    // |0| -> hasMetallic is false
    // |1| -> hasEmission  is true
    // set flags true with flags |= flag for true,
    // and &= flag for false.
    // example: m_data |= hasAbledo; //set hasAbledo to true
    enum _MaterialTextureBitmask : bitmask {
        hasAlbedo                  = 1 << 0,
        hasNormal                  = 1 << 1,
        hasMetallic                = 1 << 2,
        hasEmission                = 1 << 3
    };

    class Material : ICachable<Material> {
    protected:

        // texture pointers
        std::shared_ptr<Texture2D> m_albedoTx;
        std::shared_ptr<Texture2D> m_normalTx;
        std::shared_ptr<Texture2D> m_metallicTx;
        std::shared_ptr<Texture2D> m_emissionTx;

        // shader storage buffer object !!!!!!!!!!!
        static GLuint m_ssbo;
        // has this function done generation of the ssbo?
        bool m_generated = false;

        // generate the ssbo 
        void _genMaterial();
        
    public:

        // this will run the _genMaterial() function.
        // also return a pointer to this instance.
        static std::shared_ptr<Material> create();

        // yeah
        const GLuint getSSBO() const { return m_ssbo; }
        const bool isGenerated() const { return m_generated; }

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
        struct PBRData {
            fm::vec4 m_albedoFac     = {1.0f, 1.0f, 1.0f, 1.0f};
            fm::vec4 m_emissionFac   = {0.0f, 0.0f, 0.0f, 0.0f};
            float m_roughnessFac     = 0.5f;
            float m_metallicFac      = 0.0f;
            float m_ambientFac       = 1.0f;
            bitmask m_txFlags        = {0};
        } m_data;
        
        // the shader program
        std::shared_ptr<ShaderProgram> m_program;

        // use this material for drawing.
        // (bind the textures.)
        void useMaterial();

    };
}