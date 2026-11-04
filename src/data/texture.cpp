#include "gl_context.hpp"
#include "logger.hpp"
#include <texture.hpp>
#include <resources.hpp>
#include <fstream>
#include <renderer.hpp>
#include <mesh.hpp>

#define STBI_SUPPORT_ZLIB
#define STBI_SSE2
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace Engine;

std::unordered_map<GLuint, CacheID>& Texture2D::txIDToIDMap() {
    static std::unordered_map<GLuint, CacheID> map;
    return map;
}
std::unordered_map<std::string, CacheID>& Texture2D::txPathToIDMap() {
    static std::unordered_map<std::string, CacheID> map;
    return map;
}

std::shared_ptr<Texture2D> Texture2D::createRenderTexture(fm::uvec2 size, GLuint filtering, GLuint wrapping) {
    
    std::shared_ptr<Texture2D> texture = std::shared_ptr<Texture2D>(new Texture2D);
    // set the texture params
    texture->m_txType         = GL_TEXTURE_2D;
    texture->m_wrapping       = wrapping;
    texture->m_filtering      = filtering;
    texture->m_format         = GL_RGBA;
    texture->m_internalFormat = GL_RGBA8;
    texture->m_txSize         = (size != fm::uvec2::zero()) ? size : Renderer::get().getWndSize();
    texture->m_useMipmapping  = false;
    texture->m_channelCount   = 4;

    // gen the texture with no data (the framebuffer writes to it)
    texture->genTexture(nullptr); 

    return texture;
}

// use stb_image to convert our raw texture bytes into 
// a usable data buffer for opengl to draw with
unsigned char* Texture2D::_processRawBytes(bytes& bytes, std::shared_ptr<Texture2D> tx) {
    // annoying asf cause stbi_load_from_memory takes ints and not uints.
    // its whatever i guess.
    int x, y, c;
    
    // get the image data
    auto out = stbi_load_from_memory(bytes.data(), static_cast<int>(bytes.size()), &x, &y, &c, 4);
    
    // perform a cheeky null check on the image data
    if (!out) {
        log().logError("Texture read error! -> {}", stbi_failure_reason());
        return NULL;
    }
    // set some texture instance variables while we're here
    tx->m_channelCount = c;
    tx->m_txSize = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
    
    return out;
}

std::shared_ptr<Texture2D> Texture2D::createTextureFromAi(aiMaterial* mat, const aiScene* scene, TextureType type) {
    
    // to store the out path
    aiString aiPath;
    std::string path;

    // convert our TextureType enum to assimp's
    aiTextureType t;
    switch (type) {
        case TextureType::Albedo:   t = aiTextureType_BASE_COLOR; break;
        case TextureType::Normal:   t = aiTextureType_NORMALS;    break;
        case TextureType::Metallic: t = aiTextureType_METALNESS;  break;
        case TextureType::Emission: t = aiTextureType_EMISSIVE;   break;
        default:                    return nullptr;
    }
    // try get the path
    if (mat->GetTexture(t, 0, &aiPath) == AI_SUCCESS) {
        path = aiPath.C_Str();
    }
    else {
        // attempt fallback texture types
        if (type == TextureType::Albedo && mat->GetTexture(aiTextureType_DIFFUSE, 0, &aiPath) == AI_SUCCESS) {
            path = aiPath.C_Str();
        }
        else if (type == TextureType::Metallic && mat->GetTexture(aiTextureType_UNKNOWN, 0, &aiPath) == AI_SUCCESS) {
            path = aiPath.C_Str();
        } 
        else {
            // doesn't exist.
            return nullptr; 
        }
    }

    // if this texture was loaded already, just load that.
    if (auto tx = getTextureViaPath(path); tx != nullptr) {
        return tx;
    }

    // old way failed, attempt modern way. 
    auto embeddedTx = scene->GetEmbeddedTexture(path.c_str());
    if (embeddedTx != nullptr) {
        // yes! create the texture
        auto tx = std::shared_ptr<Texture2D>(new Texture2D());
        tx->m_path = path;

        // check if this image is compressed or not
        bool isCompressed = (embeddedTx->mHeight == 0);

        
        // get the total byte size of the texture
        size_t byteSize = isCompressed ? 
            static_cast<size_t>(embeddedTx->mWidth) : 
            static_cast<size_t>(embeddedTx->mWidth * embeddedTx->mHeight * sizeof(aiTexel));
        
        unsigned char* bytes;

        if (isCompressed) {
            int x, y, c;
            bytes = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(embeddedTx->pcData), byteSize, &x, &y, &c, 4);
            tx->m_txSize = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
            tx->m_channelCount = c;
        } 
        else {

            // not compressed, we can just make the texture as normal
            tx->m_txSize = {embeddedTx->mWidth, embeddedTx->mHeight};
            tx->m_channelCount = 4;
        }

        tx->_initTextureWithData(bytes, true);
        txPathToIDMap().emplace(tx->m_path, tx->getCacheID());
        return tx;
    }


    // definitely not embedded, load it normally.
    return createTextureFromPath(path);
    
}

// unused
EmbeddedTextureData Texture2D::getEmbeddedTexture(const aiScene* scene, int index) {

    EmbeddedTextureData out;

    // check if index is valid
    if (index < 0 || index >= static_cast<int>(scene->mNumTextures)) {
        log().logError("Embedded texture index '{}' out of bounds.", index);
        return out;
    }

    // get the texture pointer
    const aiTexture* texture = scene->mTextures[index];

    // check if texture is compressed
    if (texture->mHeight == 0) {
        // it is compressed

        // get the start of the texture byte data in memory
        const unsigned char* start = reinterpret_cast<const unsigned char*>(texture->pcData);
        // copy the byte data to a vector (bytes is a uchar vector)
        out.m_data = bytes(start, start + texture->mWidth);
        // format hint/extension
        out.m_formatHint = texture->achFormatHint;
        // texture is compressed.
        out.m_isCompressed = true;
    } else {

        // get the total byte size of the texture
        size_t byteSize = texture->mWidth * texture->mHeight * sizeof(aiTexel);
        // get the start of the texture byte data in memory
        const unsigned char* start = reinterpret_cast<const unsigned char*>(texture->pcData);
        // copy the byte data to a vector (bytes is a uchar vector)
        out.m_data = bytes(start, start + byteSize);
        out.m_size = {texture->mWidth, texture->mHeight};
    }

    return out;
}

// create a texture from a path.
std::shared_ptr<Texture2D> Texture2D::createTextureFromPath(std::filesystem::path path, bool flipY, bool mipmap, GLuint format, GLuint filtering, GLuint internalFormat, GLuint wrapping, GLuint type) {

    // check if this texture is loaded already
    if (auto tx = getTextureViaPath(path.string()); tx != nullptr) {
        return tx;
    }
    // no existing texture.

    stbi_set_flip_vertically_on_load(flipY);
    std::shared_ptr<Texture2D> texture = std::shared_ptr<Texture2D>(new Texture2D);
    texture->m_path = path.string();
    // TODO : In case of error, return a default "error" texture instead.
    
    // check if the file is in an archive first
    bytes rcFile = ResourceManager::loadRaw(path.string()); 
    if (rcFile.size() > 0) {

        // process our raw bytes with stb_image.h.
        // when we use loadRaw to read the image,
        // the ResourceManager just returns
        // the exact byte sequence of the file.
        // this means that pngs and jpgs and stuff
        // will read as completely different data.
        // stb_image.h is a small header library
        // that handles image data like this, 
        // and converts it to raw R-G-B-A-R-G-B-A data.
        auto bytes = _processRawBytes(rcFile, texture);
        if (!bytes) {
            return nullptr;
        }

        // initialise texture with stbi data
        texture->_initTextureWithData(bytes, false, mipmap, format, filtering, internalFormat, wrapping, type);
        txPathToIDMap().emplace(texture->m_path, texture->getCacheID());
        return texture;
    }
    // file not in an archive.

     // try read the file 
    std::error_code ec;    
    if (!std::filesystem::exists(path, ec)) {
        // file read failed, print error message.
        log().logError("Texture '{}' file read error! -> {}", path.string(), ec.message());
        return nullptr;
    }
    // file read success!

    int x, y, c;
    auto bytes = stbi_load(path.string().c_str(), &x, &y, &c, 4);
    texture->m_channelCount = c;
    texture->m_txSize = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};

    // initialise texture with stbi data
    texture->_initTextureWithData(bytes, false, mipmap, format, filtering, internalFormat, wrapping, type);
    txPathToIDMap().emplace(texture->m_path, texture->getCacheID());
    return texture;
}

void Texture2D::_initTextureWithData(const unsigned char* data, bool dontfree, bool mipmap, GLuint format, GLuint filtering, GLuint internalFormat, GLuint wrapping, GLuint type) {

    // set the texture values
    m_format            = format;
    m_internalFormat    = internalFormat;
    m_filtering         = filtering;
    m_wrapping          = wrapping;
    m_txType            = type;
    m_useMipmapping     = mipmap;

    // generate the texture.
    genTexture(data);

    if (!dontfree) {
        // free the data, we are done with it.
        stbi_image_free(const_cast<unsigned char*>(data));

    }
    // register this texture with the ObjectRegistry
    _register();
}

void Texture2D::genTexture(const unsigned char* data) {

    // TODO : incorporate pixel buffer objects
    // for videos later. check if the image is static.
    // if the image is NOT static, (video) then use the PBO.

    // set the minification filter.
    // if using mipmapping, account for that.
    GLuint mini_filter; 
    if (m_useMipmapping) {
        mini_filter = (m_filtering == GL_NEAREST) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
    }
    else {
        mini_filter = m_filtering;
    }

    // if this texture has been generated already,
    // then we are regenerating it. so delete the
    // old one first.
    if (m_txGenerated) glDeleteTextures(1, &m_txID);

    // packs differently if the texture is single channel idfk why this works
    // https://github.com/fayeknd/peachmilk_editor/blob/main/src/render/texture.cpp
    // i did it here too so im trusting myself
    if (m_channelCount == 1) glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // actually generate the texture
    glGenTextures(1, &m_txID);

    if (!m_txGenerated) txIDToIDMap().emplace(m_txID, this->getCacheID()); // register this to the txIDtoID map
    glBindTexture(m_txType, m_txID);

    // set the texture parameters
    glTexParameteri(m_txType, GL_TEXTURE_WRAP_S, m_wrapping);
    glTexParameteri(m_txType, GL_TEXTURE_WRAP_T, m_wrapping);
    glTexParameteri(m_txType, GL_TEXTURE_MIN_FILTER, mini_filter);
    glTexParameteri(m_txType, GL_TEXTURE_MAG_FILTER, m_filtering);
    
    //                                                                                         // data might not be set, account for that
    glTexImage2D(m_txType, 0, m_internalFormat, m_txSize.x, m_txSize.y, 0, m_format, GL_UNSIGNED_BYTE, (data != nullptr) ? data : NULL);
    if (m_useMipmapping) {
        glGenerateMipmap(m_txType);
    }
    // assign the texture data

    // unbind this texture.
    glBindTexture(m_txType, 0);

    // texture has been generated!
    m_txGenerated = true;

}

std::shared_ptr<Texture2D> Texture2D::getTextureViaID(GLuint tid) {
    auto it = txIDToIDMap().find(tid);
    if (it != txIDToIDMap().end()) {
        
        // got a cache id, try get an instance 
        auto inst = ICachable::getCachedInstance(it->second);
        if (!inst) {
            // instance is null, texture must have been destroyed or is invalid.
            // delete this entry from the map
            txIDToIDMap().erase(it);
            return nullptr;
        }
        return inst;
    }
    // no texture with that key in cache.
    return nullptr;
}

std::shared_ptr<Texture2D> Texture2D::getTextureViaPath(std::string tpath) {
    auto it = txPathToIDMap().find(tpath);
    if (it != txPathToIDMap().end()) {
        
        // got a cache id, try get an instance 
        auto inst = ICachable::getCachedInstance(it->second);
        if (!inst) {
            // instance is null, texture must have been destroyed or is invalid.
            // delete this entry from the map
            txPathToIDMap().erase(it);
            return nullptr;
        }
        return inst;
    }
    // no texture with that key in cache.
    return nullptr;
}

// add this texture to the queue and mark inQueue as true
void Texture2D::_queue() {
    auto& txQueue = GLContext::get().glState.m_txQueue;
    txQueue.push(m_txID);

    // get the next free unit id
    m_unitID = s_unitPool.getNewID();
    GLuint unit = GL_TEXTURE0 + m_unitID;
    // active texture is 0 + the unit id
    glActiveTexture(unit);
    GLContext::get().glState.m_activeTextureUnit = unit;
    glBindTexture(m_txType, m_txID);

    m_inQueue = true;
}

void Texture2D::_unqueue() {
    // just pops the front, but since this is 
    // being called on the front object anyway,
    // that's fine.
    GLContext::get().glState.m_txQueue.pop();
    m_inQueue = false;
    // let the ID pool know that this isn't
    // an active texture anymore.
    s_unitPool.releaseID(m_unitID);
}

void Texture2D::bindTexture() {
    if (m_invalid) return;
    if (m_inQueue) {
        // set active texture to this
        GLuint unit = GL_TEXTURE0 + m_unitID;
        glActiveTexture(unit);
        GLContext::get().glState.m_activeTextureUnit = unit;
        
        glBindTexture(m_txType, m_txID); 
        return;
    }
    auto& txQueue = GLContext::get().glState.m_txQueue; 

    if (txQueue.size() >= GLContext::get().maxTextureSlots()) {
        // tell the texture associated with this textureid to remove itself from the queue.
        auto tx = getTextureViaID(txQueue.front()); 
        if (tx != nullptr) {
            // unqueue properly
            tx->_unqueue();
        }
        else {
            // object was deleted.
            // if so, the unit ID has been
            // unbound already, so just
            // pop the front element.
            txQueue.pop(); 
        }
    }
    // add this to the queue
    this->_queue();
}

// destructor. delete this texture if it exists.
Texture2D::~Texture2D() {
    if (m_txGenerated) {
        glDeleteTextures(1, &m_txID);
    }
}

// set the wrapping mode 
void Texture2D::setWrapping(GLuint wrapping) {
    bindTexture();
    m_wrapping = wrapping;
    glTexParameteri(m_txType, GL_TEXTURE_WRAP_S, m_wrapping);
    glTexParameteri(m_txType, GL_TEXTURE_WRAP_T, m_wrapping);
}

// set the filtering mode
void Texture2D::setFiltering(GLuint filtering) {
    bindTexture();

    m_filtering = filtering;

    GLuint mini_filter; 
    if (m_useMipmapping) {
        mini_filter = (m_filtering == GL_NEAREST) ? GL_NEAREST_MIPMAP_NEAREST : GL_LINEAR_MIPMAP_LINEAR;
    }
    else {
        mini_filter = m_filtering;
    }

    glTexParameteri(m_txType, GL_TEXTURE_MIN_FILTER, mini_filter);
    glTexParameteri(m_txType, GL_TEXTURE_MAG_FILTER, m_filtering);
}