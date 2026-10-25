#include "component/component.hpp"
#include "gl_context.hpp"
#include "shader.hpp"
#include <material.hpp>

using namespace Engine;

// commenting one, cause the others
// are the same just with different textures.
void Material::setAlbedoTx(std::shared_ptr<Texture2D> tx) {
    
    // is texture null?
    if (!tx) {
        // yes, set the internal material texture to null
        m_albedoTx = nullptr;
        // hasAlbedo is now false
        m_data.m_hasAlbedoTx = false;
        return; // early return
    }
    // no, set the internal material texture to the provided shared_ptr
    m_albedoTx = tx;
    // hasAlbedo is now true
    m_data.m_hasAlbedoTx = true;
}
void Material::setNormalTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_normalTx = nullptr;
        m_data.m_hasNormalTx = false;
        return; 
    }
    m_normalTx = tx;
    m_data.m_hasNormalTx = true;
}
void Material::setMetallicTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_metallicTx = nullptr;
        m_data.m_hasMetallicTx = false;
        return; 
    }
    m_metallicTx = tx;
    m_data.m_hasMetallicTx = true;
}
void Material::setEmissionTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_emissionTx = nullptr;
        m_data.m_hasEmissionTx = false;
        return; 
    }
    m_emissionTx = tx;
    m_data.m_hasEmissionTx = true;
}

std::shared_ptr<Material> Material::create() {
    // create the material
    auto material = std::shared_ptr<Material>(new Material);
    // use the default lit shader
    material->m_program = ShaderProgram::getShaderViaKey(shader_litShader);
    // return our new material
    return material;
}
void Material::useMaterial() {
    
    // early return if the program is null
    if (!m_program) return;
    
    if (m_albedoTx)   m_albedoTx->bindTexture();
    if (m_normalTx)   m_normalTx->bindTexture();
    if (m_metallicTx) m_metallicTx->bindTexture();
    if (m_emissionTx) m_emissionTx->bindTexture();

    m_program->setUniform(shader_defaultMaterial, this);

}

