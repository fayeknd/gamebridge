#include "component/component.hpp"
#include "gl_context.hpp"
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
        m_data.m_txFlags &= hasAlbedo;
        return; // early return
    }
    // no, set the internal material texture to the provided shared_ptr
    m_albedoTx = tx;
    // hasAlbedo is now true
    m_data.m_txFlags |= hasAlbedo;
}
void Material::setNormalTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_normalTx = nullptr;
        m_data.m_txFlags &= hasNormal;
        return; 
    }
    m_normalTx = tx;
    m_data.m_txFlags |= hasNormal;
}
void Material::setMetallicTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_metallicTx = nullptr;
        m_data.m_txFlags &= hasMetallic;
        return; 
    }
    m_metallicTx = tx;
    m_data.m_txFlags |= hasMetallic;
}
void Material::setEmissionTx(std::shared_ptr<Texture2D> tx) {
    if (!tx) {
        m_emissionTx = nullptr;
        m_data.m_txFlags &= hasEmission;
        return; 
    }
    m_emissionTx = tx;
    m_data.m_txFlags |= hasEmission;
}

std::shared_ptr<Material> Material::create() {
    auto material = std::shared_ptr<Material>(new Material);
    material->_genMaterial();
    return material;
}

void Material::_genMaterial() {

    glGenBuffers(1, &m_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);

    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(PBRData), m_data,)

    // register this material in the ObjectRegistry
    this->_register();
}

void Material::useMaterial() {
    if (m_albedoTx)   m_albedoTx->bindTexture();
    if (m_normalTx)   m_normalTx->bindTexture();
    if (m_metallicTx) m_metallicTx->bindTexture();
    if (m_emissionTx) m_emissionTx->bindTexture();

}

