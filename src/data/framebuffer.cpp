#include <framebuffer.hpp>
#include <renderer.hpp>

using namespace Engine;

void Framebuffer::_genBuffer(fm::uvec2 size) {

    if (m_initialised) _deleteBuffer(); // we are changing some params    

    // make the framebuffer
    glGenFramebuffers(1, &m_bufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);

    if (size == fm::uvec2::zero()) { // no size was passed, assume window size.
        size = Renderer::get().getWndSize();
    }
    // the user may have passed getWndSize directly.
    if (size == Renderer::get().getWndSize()) {
        m_resizeWithWnd = true; // if window size, we should probably resize it when the window size changes.
    }
    m_size = size;

    // to make it clear - this should not be used to resize the framebuffer.
    // it CAN be used, but it's slow. if you want to resize the framebuffer
    // properly, just use fb->setSize(fm::uvec2).
    if (!m_renderTexture) {
        // create the render texture we will read from
        m_renderTexture = Texture2D::createRenderTexture(size);
    }
    else {
        // texture already exists, we are likely
        m_renderTexture->m_txSize = m_size;
        m_renderTexture->genTexture(nullptr);
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderTexture->getTextureType(), m_renderTexture->getTextureID(), 0);


    // make the render buffer
    glGenRenderbuffers(1, &m_renderBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, m_format, m_renderTexture->getSize().x, m_renderTexture->getSize().y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferID);

    // sanity check on the framebuffer.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log().logError("Framebuffer creation failed!");
        _deleteBuffer(); // free the buffers; they aren't doing anything. 
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::shared_ptr<Framebuffer> Framebuffer::create(fm::uvec2 size, GLuint format) {
    
    // make our framebuffer
    auto fb = std::shared_ptr<Framebuffer>(new Framebuffer());
    // generate the framebuffer
    fb->_genBuffer(size);
    // register it to the objectregistry
    fb->_register();
    // get a new framebuffer id
    fb->m_framebufferID = framebufferIDPool().getNewID();
    // set the format
    fb->m_format = format;
    // add the framebuffer id to the map
    framebufferMap().emplace(fb->m_framebufferID, fb);

    log().logInfo("Created new Framebuffer with size {} and ID {}.", fb->m_size, fb->m_bufferID);

    return fb;
}

void Framebuffer::setSize(fm::uvec2 size) {
    if (m_size == size) return;

    if (size == fm::uvec2::zero()) { // no size was passed, assume window size.
        size = Renderer::get().getWndSize();
    }
    // the user may have passed getWndSize directly.
    if (size == Renderer::get().getWndSize() && !m_resizeWithWnd) {
        m_resizeWithWnd = true; // if window size, we should probably resize it when the window size changes.
    }

    // temporarily store the old id
    GLint old;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &old);
    // bind the render texture
    glBindTexture(GL_TEXTURE_2D, m_renderTexture->getTextureID());
    // re-set the texture data.
    // pass null here because we aren't actually initialising
    // it with any data, we are writing to this texture.
    glTexImage2D(GL_TEXTURE_2D, 0, m_renderTexture->m_internalFormat, size.x, size.y, 0, m_renderTexture->m_format, GL_UNSIGNED_BYTE, NULL);

    // update the renderbuffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, m_format, size.x, size.y);

    // just in case, set the viewport. 
    // probably doesn't need to happen, 
    // but maybe the viewport is fucked up
    glViewport(0, 0, size.x, size.y); 
    m_size = size;

    // reset the bound texture to whatever it was before
    glBindTexture(GL_TEXTURE_2D, old);
}

void Framebuffer::setResizeWithWnd(bool v) {
    if (m_resizeWithWnd == v) return;
    // force framebuffer size of 0, which enables m_resizeWithWnd/
    if (v) setSize(fm::uvec2::zero());
    else   m_resizeWithWnd = v; // it won't be set, set it ourselves.
}

void Framebuffer::_deleteBuffer() {

    // delete the gl buffers
    glDeleteBuffers(1, &m_bufferID);
    glDeleteBuffers(1, &m_renderBufferID);
    // make sure the IDs are 0.
    // we do this because the framebuffer
    // might be being remade or something.
    m_bufferID = 0;
    m_renderBufferID = 0;
}

Framebuffer::~Framebuffer() {
    // free this buffer data
    _deleteBuffer();
    // erase this framebuffer from the fb map
    framebufferMap().erase(m_framebufferID);
    // release the framebuffer id
    framebufferIDPool().releaseID(m_framebufferID);
}

void Framebuffer::bindFramebuffer() {
    // check if this fb is already bound
    if (GLContext::get().glState.m_currentFramebuffer == m_bufferID) return;

    GLContext::get().glState.m_currentFramebuffer = m_bufferID;
    // bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);
    
    // set the viewport to the framebuffer size.
    glViewport(0, 0, m_size.x, m_size.y);
}