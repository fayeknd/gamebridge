#include "gl_context.hpp"
#include "logger.hpp"
#include <texture.hpp>
#include <resources.hpp>
#include <fstream>
#include <renderer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

using namespace Engine;

std::unordered_map<GLuint, CacheID>& Texture2D::txIDToIDMap() {
    static std::unordered_map<GLuint, CacheID> map;
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
bool Texture2D::_processRawBytes(bytes &outBytes, std::shared_ptr<Texture2D> tx) {
    // annoying asf cause stbi_load_from_memory takes ints and not uints.
    // its whatever i guess.
    int x, y, c;
    
    // get the image data
    auto imgData = stbi_load_from_memory(outBytes.data(), static_cast<int>(outBytes.size()), &x, &y, &c, 4);
    
    // perform a cheeky null check on the image data
    if (!imgData) {
        log().logError("Texture read error! -> {}", stbi_failure_reason());
        return false;
    }
    // convert back to a vector 
    size_t imgTotalBytes = static_cast<size_t>(x) * y * 4;
    bytes pixels(imgData, imgData + imgTotalBytes);
    outBytes = pixels;

    // set some texture instance variables while we're here
    tx->m_channelCount = c;
    tx->m_txSize = {static_cast<unsigned int>(x), static_cast<unsigned int>(y)};
    
    // free the data, we are done with it.
    stbi_image_free(imgData);

    return true;
}

// create a texture from a path.
std::shared_ptr<Texture2D> Texture2D::createTextureFromPath(std::filesystem::path path, bool flipY, bool mipmap, GLuint format, GLuint filtering, GLuint internalFormat, GLuint wrapping, GLuint type) {

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
        if (!_processRawBytes(rcFile, texture)) {
            return nullptr;
        }

        // initialise texture with stbi data
        texture->_initTextureWithSTBIData(rcFile, mipmap, format, filtering, internalFormat, wrapping, type);
        return texture;
    }
    // file not in an archive.

     // try read the file 
    std::error_code ec;    
    if (!std::filesystem::exists(path, ec)) {
        // file read failed, print error message.
        log().logError("Texture file read error! -> {}", ec.message());
        return nullptr;
    }
    // file read success!

    // open the file
    std::ifstream file(path);

    // read file contents into a buffer
    std::stringstream buf;
    buf << file.rdbuf();
    // close file. data is in the buffer now.
    file.close();

    // convert stringstream buffer to uchar vector buffer
    std::string bufstr = buf.str();
    bytes data(bufstr.begin(), bufstr.end());

    if (!_processRawBytes(data, texture)) {
        return nullptr;
    }

    // initialise texture with stbi data
    texture->_initTextureWithSTBIData(data, mipmap, format, filtering, internalFormat, wrapping, type);
    return texture;
}

void Texture2D::_initTextureWithSTBIData(bytes &data, bool mipmap, GLuint format, GLuint filtering, GLuint internalFormat, GLuint wrapping, GLuint type) {

    // set the texture values
    m_format            = format;
    m_internalFormat    = internalFormat;
    m_filtering         = filtering;
    m_wrapping          = wrapping;
    m_txType            = type;
    m_useMipmapping     = mipmap;

    // generate the texture.
    genTexture(&data);

    // register this texture with the ObjectRegistry
    _register();
}

void Texture2D::genTexture(bytes* data) {

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
    glTexImage2D(m_txType, 0, m_internalFormat, m_txSize.x, m_txSize.y, 0, m_format, GL_UNSIGNED_BYTE, (data != nullptr) ? data->data() : NULL);
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
    if (m_unitID) s_unitPool.releaseID(m_unitID);
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