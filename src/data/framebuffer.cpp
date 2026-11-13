#include <framebuffer.hpp>
#include <renderer.hpp>

using namespace Engine;

void Framebuffer::_genBuffer(fm::uvec2 size) {

    if (m_initialised) _deleteBuffer(); // we are changing some params    

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
    if (!m_pingRenderTexture || !m_pongRenderTexture) {
        // create the render textures we will read from
        m_pingRenderTexture = Texture2D::createRenderTexture(size);
        m_pongRenderTexture = Texture2D::createRenderTexture(size);
    }
    else {
        // textures already exists, we are likely remaking the fb
        m_pingRenderTexture->m_txSize = m_size;
        m_pingRenderTexture->genTexture(nullptr);
        
        m_pongRenderTexture->m_txSize = m_size;
        m_pongRenderTexture->genTexture(nullptr);
    }

    // generate buffers
    glGenFramebuffers(1, &m_bufferID);
    glGenRenderbuffers(1, &m_renderBufferID);

    // make the ping framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);
    
    // add the render textures 
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_pingRenderTexture->getTextureType(), m_pingRenderTexture->getTextureID(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, m_pongRenderTexture->getTextureType(), m_pongRenderTexture->getTextureID(), 0);

    // make the render buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, m_format, m_size.x, m_size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferID);

    // sanity check on the framebuffer.
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        log().logError("Framebuffer creation failed!");
        _deleteBuffer(); // free the buffers; they aren't doing anything. 
    }

    m_initialised = true;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::shared_ptr<Framebuffer> Framebuffer::create(fm::uvec2 size, GLuint format) {
    
    // make our framebuffer
    auto fb = std::shared_ptr<Framebuffer>(new Framebuffer());
    // generate the framebuffer
    fb->_genBuffer(size);
    // register it to the objectregistry
    fb->_register();
    // get a new framebuffer map id
    fb->m_fbMapID = framebufferIDPool().getNewID();
    // set the format
    fb->m_format = format;
    // add the framebuffer id to the map
    framebufferMap().emplace(fb->m_fbMapID, fb);

    // create a postprocess material
    fb->m_postProcessMaterial = Material::create();
    // set the mat albedo tx to the fb render texture
    fb->m_postProcessMaterial->setAlbedoTx(fb->m_pingRenderTexture);

    log().logInfo("Created new Framebuffer with size {} and ID {}. (CacheID: {})", fb->m_size, fb->m_fbMapID, fb->getCacheID());

    return fb;
}

bool Framebuffer::setSize(fm::uvec2 size) {
    if (m_size == size) return false;

    if (size == fm::uvec2::zero()) { // no size was passed, assume window size.
        size = Renderer::get().getWndSize();
    }
    // the user may have passed getWndSize directly.
    if (size == Renderer::get().getWndSize() && !m_resizeWithWnd) {
        m_resizeWithWnd = true; // if window size, we should probably resize it when the window size changes.
    }

    // set the size for both textures
    _setSpecificTexSize(size, m_pingRenderTexture);
    _setSpecificTexSize(size, m_pongRenderTexture);
    
    // just in case, set the viewport. 
    // probably doesn't need to happen, 
    // but maybe the viewport is fucked up
    glViewport(0, 0, size.x, size.y); 
    m_size = size;

    return true;
}

void Framebuffer::_setSpecificTexSize(fm::uvec2 size, std::shared_ptr<Texture2D> tex) {
    tex->bindTexture();
    // re-set the texture data.
    // pass null here because we aren't actually initialising
    // it with any data, we are writing to this texture.
    glTexImage2D(GL_TEXTURE_2D, 0, tex->m_internalFormat, size.x, size.y, 0, tex->m_format, GL_UNSIGNED_BYTE, NULL);
    // update the renderbuffer(s)
    glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferID);
    glRenderbufferStorage(GL_RENDERBUFFER, m_format, size.x, size.y);

    tex->m_txSize = size;
}

void Framebuffer::setResizeWithWnd(bool v) {
    if (m_resizeWithWnd == v) return;
    // force framebuffer size of 0, which enables m_resizeWithWnd/
    if (v) setSize(fm::uvec2::zero());
    else   m_resizeWithWnd = v; // it won't be set, set it ourselves.
}

void Framebuffer::_deleteBuffer() {

    if (!m_initialised) return;
    m_initialised = false;

    // BULLSHIT NVIDIA BUG
    // if a vertex array is bound when
    // the fb is deleted, OPENGL CRASHES!
    glBindVertexArray(0); 

    // delete the gl buffers
    glDeleteBuffers(1, &m_bufferID);
    glDeleteBuffers(1, &m_renderBufferID);

    // rebind the stupid mesh
    glBindVertexArray(GLContext::get().glState.m_currentMesh);
    // make sure the IDs are 0.
    // we do this because the framebuffer
    // might be being remade or something.
    m_bufferID = 0;
    m_renderBufferID = 0;

}

Framebuffer::~Framebuffer() {

    FramebufferID id = m_fbMapID;
    // erase this framebuffer from the fb map
    framebufferMap().erase(id);
    // release the framebuffer id
    framebufferIDPool().releaseID(id);
    // set the current framebuffer to 0 if this fb is bound
    auto& fb = GLContext::get().glState.m_currentFramebuffer;
    if (fb == m_bufferID) {
        fb = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    // remove the ping render texture from the texture queue
    if (m_pingRenderTexture->m_inQueue) {
        m_pingRenderTexture->m_invalid = true;
    }
    // remove the pong render texture from the texture queue
    if (m_pongRenderTexture->m_inQueue) {
        m_pongRenderTexture->m_invalid = true;
    }
    // free this buffer data
    _deleteBuffer();
}

void Framebuffer::bindFramebuffer(bool pong) {

    // check if this fb is already bound
    if (GLContext::get().glState.m_currentFramebuffer == m_bufferID) return;

    // set glstate to this buffer id
    GLContext::get().glState.m_currentFramebuffer = m_bufferID;

    // bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_bufferID);

    // set the viewport to the framebuffer size.
    glViewport(0, 0, m_size.x, m_size.y);
}

// unbind any currently bound framebuffers 
void Framebuffer::unbindFramebuffers() {
    if (GLContext::get().glState.m_currentFramebuffer == 0) return;

    // set glstate to framebuffer 0 (default fb)
    GLContext::get().glState.m_currentFramebuffer = 0;

    // bind framebuffer 0
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // set the viewport to the screen size.
    glViewport(0, 0, Renderer::get().getWndSize().x, Renderer::get().getWndSize().y);
}

void Framebuffer::doPostProcess() {
    // don't do post-processing if there is no post-process shader
    if (m_postProcessSteps.empty()) return;

    // disable depth testing, to not discard any of the plane's fragments
    glDisable(GL_DEPTH_TEST);

    // loop through all the post process steps
    for (size_t i = 0; i < m_postProcessSteps.size(); i++) {
        // determines if this is using the ping texture or pong texture. 
        bool isEven = (i % 2 == 0);

        // get the read texture to use for this post process step
        auto readTexture = (i % 2 == 0) ? m_pingRenderTexture : m_pongRenderTexture;

        // tell opengl which write attachment to draw with
        glDrawBuffer(isEven ? GL_COLOR_ATTACHMENT1 : GL_COLOR_ATTACHMENT0);

        // perform post process step
        _doPostProcessImpl(m_postProcessSteps[i], readTexture);
    }

    // if the size is odd, we ended on pong. so copy the texture back to ping.
    if (m_postProcessSteps.size() % 2 != 0) {
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        m_pingRenderTexture->bindTexture();
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_pingRenderTexture->m_txSize.x, m_pingRenderTexture->m_txSize.y);
    }

    // set read/draw buffer back
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
}

void Framebuffer::_doPostProcessImpl(std::shared_ptr<ShaderProgram> postprocess, std::shared_ptr<Texture2D> tex) {
    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // set the texture and postprocess step
    m_postProcessMaterial->m_program = postprocess;
    m_postProcessMaterial->setAlbedoTx(tex);

    // draw a primitive with the postprocess material
    Model::primitive(Primitive::InvertedPlane)->draw(m_postProcessMaterial);
}

std::shared_ptr<Texture2D> Framebuffer::getRenderTexture() {
    //return m_pingPongState ? m_pongRenderTexture : m_pingRenderTexture;
    return m_pingRenderTexture;
}