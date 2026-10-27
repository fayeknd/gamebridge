#include <framebuffer.hpp>
#include <renderer.hpp>

using namespace Engine;

void Framebuffer::_genBuffer(fm::uvec2 size) {

    if (m_initialised) _deleteBuffer(); // we are changing some params    

    // make the framebuffer
    glGenFramebuffers(1, &m_bufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);

    if (size == fm::uvec2::zero()) { // no size was passed, assume window size.
        m_resizeWithWnd = true; // if window size, we should probably resize it when the window size changes.
        size = Renderer::get().getWndSize();
    }
    m_size = size;

    // create the render texture we will read from
    m_renderTexture = Texture2D::createRenderTexture(size);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_renderTexture->getTextureType(), m_renderTexture->getTextureID(), 0);


    // make the render buffer
    glGenRenderbuffers(1, &m_renderBufferID);
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_renderTexture->getSize().x, m_renderTexture->getSize().y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferID);

    // sanity check on the framebuffer.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log().logError("Framebuffer creation failed!");
        _deleteBuffer(); // free the buffers; they aren't doing anything. 
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::shared_ptr<Framebuffer> Framebuffer::create(fm::uvec2 size) {
    
    // make our framebuffer
    auto fb = std::shared_ptr<Framebuffer>(new Framebuffer());
    // generate the framebuffer
    fb->_genBuffer(size);
    // register it to the objectregistry
    fb->_register();
    // get a new framebuffer id
    fb->m_framebufferID = framebufferIDPool().getNewID();
    // add the framebuffer id to the map
    framebufferMap().emplace(fb->m_framebufferID, fb);

    log().logInfo("Created new Framebuffer with size {} and ID {}.", fb->m_size, fb->m_bufferID);

    return fb;
}

void Framebuffer::setSize(fm::uvec2 size) {
    if (m_size == size) return;
    // delete the buffer first
    _deleteBuffer();
    // generate a new buffer with the new size
    _genBuffer(size);
}

void Framebuffer::setResizeWithWnd(bool v) {
    if (m_resizeWithWnd == v) return;
    // force framebuffer size of 0, which enables m_resizeWithWnd/
    if (v) setSize(fm::uvec2::zero());
    else   m_resizeWithWnd = v; // it won't be set, set it ourselves.
}

void Framebuffer::_deleteBuffer() {

    // set the texture to a nullptr
    m_renderTexture = nullptr;
    // delete the gl buffers
    glDeleteBuffers(1, &m_bufferID);
    glDeleteBuffers(1, &m_renderBufferID);
    m_bufferID = 0;
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