#include <cachable.hpp>
#include <gl_context.hpp>
#include <renderer.hpp>

#include <GLFW/glfw3native.h>
#include <resources.hpp>
#include <shader.hpp>
#include <entity.hpp>
#include <component/main_comp.hpp>
#include <time.hpp>
#include <component/camera.hpp>
#include <numeric>
#include <input.hpp>
#include <material.hpp>
#include <debug.hpp>
#include <thread>
#include <physics/physics.hpp>
#include <layer.hpp>
#include <framebuffer.hpp>

using namespace Engine;

// returns the glfw Renderer handle. (different from the HWND Renderer handle.)
GLFWwindow* Renderer::getWndHandle() const {
    return m_wnd;
}

// returns the Renderer size.
fm::uvec2 Renderer::getWndSize() {
    return m_wndSize;
}

// sets the Renderer size via glfwSetRendererSize(m_wnd, v.x, v.y).
void Renderer::setWndSize(fm::uvec2 v) {
    m_wndSize = v;
    if (GLContext::get().isGLReady()) glfwSetWindowSize(m_wnd, v.x, v.y);
}

// returns the Renderer title. 
std::string Renderer::getWndTitle() {
    return m_wndTitle;
}

// sets the Renderer title via glfwSetRendererTitle(m_wnd, s.c_str()).
void Renderer::setWndTitle(std::string s) {
    m_wndTitle = s;
    if (GLContext::get().isGLReady()) glfwSetWindowTitle(m_wnd, s.c_str());
}

// returns the current Renderer fullscreen state.
FullscreenMode Renderer::getFullscreenMode() {
    return m_fsMode;
}

// sets the Renderer state via a switch statement. 
void Renderer::setFullscreenMode(FullscreenMode fm) {

    // if the current FullscreenMode is already this, just return early.
    if (fm == m_fsMode) return; 
    switch (fm) {
        case FullscreenMode::Fullscreen: { // swap to fullscreen 

            // get current width and height
            int w, h;
            glfwGetWindowSize(m_wnd, &w, &h);
            
            // cache width and height to old size
            m_wndSize_old = fm::uvec2(w, h);

            // get current x and y position of the Renderer
            int x, y;
            glfwGetWindowPos(m_wnd, &x, &y);

            // cache the x and y.
            m_wndPos_old = fm::uvec2(x, y);

            // get the monitor video mode for the width and height and refresh of the main monitor to fullscreen to.
            const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            // set the Renderer to fullscreen on the primary monitor with the OS video mode current options.
            glfwSetWindowMonitor(m_wnd, glfwGetPrimaryMonitor(), 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate); 
            break; }

        case FullscreenMode::Renderered: { // swap to Renderered
            
            // if swapping from fullscreen, use the cached old variables to restore Renderer to original size and pos.
            if (m_fsMode == FullscreenMode::Fullscreen) {

                glfwSetWindowMonitor(m_wnd, NULL, m_wndPos_old.x, m_wndPos_old.y, m_wndSize_old.x, m_wndSize_old.y, 0);
                break;
            }

            // GLFW_DECORATED determines if the Renderer borderless or not.
            // true means it is not borderless, false means it is.
            // it could be set to false, so change it back just in case.

            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_TRUE);
            break; } 

            // Don't know if this works on Renderers, should do
        case FullscreenMode::BorderlessRenderered: { // swap to borderless Renderered

            // this isn't needed on Renderers but it is on linux. hide the Renderer to make quick changes then show it again.
            glfwHideWindow(m_wnd);

            // set the decorated attrib
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_FALSE);

            // update Renderer
            glfwPollEvents();

            // show the Renderer again
            glfwShowWindow(m_wnd);

            // make sure it's focused
            glfwFocusWindow(m_wnd);
            
            // this is borderless Renderered, so make sure the Renderer is not maximised. 
            // glfwMaximiseRenderer() is named weirdly. if the Renderer is maximised when this is run,
            // it will un-maximise the Renderer.
            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_FALSE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; }

        case FullscreenMode::Borderless: {

            // same as above, just read that
            glfwHideWindow(m_wnd);
            glfwSetWindowAttrib(m_wnd, GLFW_DECORATED, GLFW_FALSE);
            glfwPollEvents();
            glfwShowWindow(m_wnd);
            glfwFocusWindow(m_wnd);

            // also same as before, but check for the other case.
            if (glfwGetWindowAttrib(m_wnd, GLFW_MAXIMIZED) == GLFW_TRUE) {
                glfwMaximizeWindow(m_wnd);
            }
            break; } 
    }
    // make sure the Renderer fullscreenmode is equal to the new fullscreenmode.
    m_fsMode = fm;
}

// get the target fps
uint16_t Renderer::getTargetFPS() {
    return m_targetFramesPerSecond;
}

uint16_t Renderer::getFPS() {
    return m_currentFramesPerSecond;
}

// get the target frame delta
double Renderer::getTargetFrameDelta() {
    return m_targetFrameDelta;
}

// set the target fps.
void Renderer::setTargetFPS(uint16_t fps) {
    m_targetFramesPerSecond = fps;
    // also calculate the new target delta.
    m_targetFrameDelta = static_cast<double>(1) / static_cast<double>(fps);
}

// set the target delta time.
void Renderer::setTargetFrameDelta(double delta) {
    m_targetFrameDelta = delta;
    // also calculate the new target fps.
    m_targetFramesPerSecond = static_cast<double>(1) / static_cast<double>(delta);
}

// get the current vsync mode
VSyncMode Renderer::getVSyncMode() {
    return m_vsyncMode;
}

// set the current vsync mode
void Renderer::setVSyncMode(VSyncMode vsm) {
    m_vsyncMode = vsm;
    glfwSwapInterval(static_cast<int>(vsm));
}

fm::uvec2 Renderer::getFractionalAspectRatio() {
    // get the greatest common divisor
    int div = std::gcd(m_wndSize.x, m_wndSize.y);
    // get ratio width
    uint8_t rWidth  = m_wndSize.x / div;
    // get ratio height
    uint8_t rHeight = m_wndSize.y / div;
    // awesome!
    return {rWidth, rHeight};
}

double Renderer::getAspectRatio() {
    // width / height = aspect
    return static_cast<double>(m_wndSize.x) / static_cast<double>(m_wndSize.y);
}

// creates the Renderer. this should be the first function to run.
// a run through of what the parameters defaults are for:

// size: the Renderer size. leave at -1 for the default value of 800x800. 
// title: Renderer title. -1 for default also.
// fsMode: fullscreen mode. by default is just Renderered.
// monitor: leave null for now.
// share: i don't really know what this does, it just gets passed through to glfwCreateRenderer.
// extra_Renderer_hints: a lambda function to add your own Renderer hints in. create a function that
// has GLFWwindowHint() functions and pass that function through as a parameter. not required.
int Renderer::createWnd(fm::uvec2 size, std::string title, FullscreenMode fsMode, GLFWmonitor *monitor, GLFWwindow *share, void(*extra_Renderer_hints)()) {

    // if opengl is ready for use, the Renderer has already been created. return early.
    if (GLContext::get().isGLReady()) return ERR_CONTEXT_EXISTS;

    // if the glfw context creation failed, also return early.
    if (GLContext::get().createGLFWContext(_default_Renderer_hints, extra_Renderer_hints) != GL_CREATE_CONTEXT_SUCCESS)
        return ERR_WINDOW_CREATION_FAILED;

    // if for whatever reason the m_wnd has already been assigned, return early. 
    if (m_wnd) return ERR_WINDOW_CREATION_FAILED;

    // create the default layer.
    LayerID layer = Layer::create(layer_defaultLayer, 0);

    // if the root entity is null, something is wrong. return.
    if (!Entity::s_root) {
        return ERR_NO_ROOT_ENTITY;
    }

    Entity::s_root->setLayer(layer);

    // check for default parameter values and set them to the true defaults accordingly.
    // the reason they are -1 is to allow setting defaults to be done with the original variables
    // in Renderer.hpp instead.
    if (size == fm::uvec2(-1)) size = m_wndSize;
    if (title == "-1") title = m_wndTitle;
    
    // actually create the Renderer. unsure what share does.
    m_wnd = glfwCreateWindow(size.x, size.y, title.c_str(), monitor, share);

    // set the Renderer variables to the passed through variables
    m_wndSize = size;
    m_wndTitle = title;
    
    // if glfwCreateRenderer failed, return.
    if (!m_wnd) {
        return ERR_WINDOW_CREATION_FAILED;
    }

    // make sure the internal opengl context is tied to this Renderer
    glfwMakeContextCurrent(m_wnd);

    // set the callback that is called whenever the Renderer changes size.
    glfwSetFramebufferSizeCallback(m_wnd, _framebuffer_resize_callback);

    // Initializes the input class
    Input::init(m_wnd);

    // if GLAD initialisation fails, return.
    if (GLContext::get().createGLADContext() != GLAD_LOAD_SUCCESS) {
        return ERR_WINDOW_CREATION_FAILED;
    }
    
    // ensure the glViewport is set to the Renderer size properly.
    glViewport(0, 0, size.x, size.y);
    m_viewportDirty = true;

    // set fullscreen mode passed in
    setFullscreenMode(fsMode);

    // set a default target fps
    setTargetFPS(60);
    
    // set the vsync mode
    setVSyncMode(VSyncMode::On);

    // create the plane to render the main framebuffer to
    m_mainFramebufferMesh = Mesh::primitive(Plane);

    // create the material for rendering the framebuffer
    m_mainFramebufferMaterial = Material::create();
    m_mainFramebufferMaterial->m_program = ShaderProgram::createProgramFromPath("stuff=>shaders/postprocess.glsl");
    m_mainFramebufferMaterial->m_data.m_hasTransparency = true;

    // create the imgui context 
    Debug::DebugUI::get()._createImGuiContext(m_wnd);

    // the max slots amount differs on GPUs. it's either 16 or 32.
    // we aren't really going to run into a gpu that only allows 16,
    // but still. may as well.
    GLint maxSlots;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxSlots); 
    GLContext::get().m_maxTextureSlots = maxSlots; 

    // create the default shaders
    ShaderProgram::_createDefaultShaders();

    // add the main entrypoint component to the root entity.
    Entity::s_root->addComponent<MainComponent>();

    // Renderer creation succeeded!!
    return WINDOW_CREATION_SUCCESS;
}

// this function defines what glfw Renderer hints are set by default.
// i.e GLFWwindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
// this is passed through to the GLContext on Renderer creation as a lambda.
void Renderer::_default_Renderer_hints() {

    // set the opengl version. in our case, OpenGL 4.6.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    // we are using the core profile. the compatability profile is really weird and old.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // doesn't really need to be done, but just specify the colour bits used for the
    // glfw main framebuffer.
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);


    // only required on linux. forces the Renderer to run under wayland instead of x11 (if available).
    #ifndef _WIN32
    glfwWindowHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);
    #endif    

    // set transparent framebuffer to true for obvious reasons.
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
}

// handles changing the m_wndSize value and glViewport.
void Renderer::_framebuffer_resize_callback(GLFWwindow* wnd, int w, int h) {

    // keep the viewport up to date
    glViewport(0, 0, w, h);
    get().m_viewportDirty = true;

    // make sure m_wndSize is aware of the updated size
    Renderer::get().m_wndSize = fm::vec<unsigned int>(w,h);

    for (const auto& fb : Framebuffer::framebufferMap()) {
        // skip if the framebuffer is null
        if (!fb.second.lock()) continue;
        // skip if the framebuffer does not resize with the windowd
        if (!fb.second.lock()->doesResizeWithWnd()) continue;

        // resize the framebuffer
        fb.second.lock()->setSize(Renderer::get().m_wndSize);
    }

}

#pragma region MainLoop

// begins the program rendering and logic loop. in main(), this should come last unless you have
// any cleanup stuff to perform after the program stops.
void Renderer::beginGameLoop() {

    // start the timer
    Time::s_deltaTimer.start();
    Timer swapBufferTimer;
    
    // while glfw believes the Renderer should not close
    while (!glfwWindowShouldClose(m_wnd)) {

        // if the delta timer is less than the target frame delta, 
        // then the frame was finished early. keep going until the 
        // delta timer is less than or equal to the target delta.
        // continue keyword means it skips everything else in the loop.

        if (Time::deltaTimeNow() < m_targetFrameDelta) {
            continue;
        }

        // reset the delta timer
        Time::_resetDeltaTimer();
        // get the delta time
        double dt = Time::deltaTime();
        // calculate fps. round it to avoid int truncation too
        m_currentFramesPerSecond = std::round(1.0 / dt);
        // poll some events including glfwPollEvents.
        preFramePollEvents(); 
        

        auto cam = Camera::activeCamera();
        // poll the Renderer events. this calls any callbacks first
        // this should have actually been done earlier but oh well.
        // without this, inputs would be handled on the NEXT frame
        // instead of this one.
        if (!cam) {
            Logging::Logger::get().logError("No active camera in game! There has to be an active camera for any behaviour to occurr.");
        }
        // poll all component behaviours
        Entity::_pollComponentBehaviours(dt);

        // moved Input::NewFrame() to postFramePollEvents 
        // (and renamed it to poll()) to clean up.
        // we might have other things that
        // have to go here so it's nicer to have them
        // all in one place.
        postFramePollEvents();

        // cycle the glfw front and back buffers
        glfwSwapBuffers(m_wnd); 
        glFinish();

        // end of frame, viewport no longer dirty
        // comes before glfwPollEvents because the 
        // framebuffer resize gets called there.
        m_viewportDirty = false;
        // increment the frame counter
        m_frame++;
    }
    // we have left the loop, so the program is attempting to close.
    cleanup();
}

#pragma endregion MainLoop

void Renderer::preFramePollEvents() {
    // poll glfw events.
    glfwPollEvents(); 
    ObjectRegistry::pollGC();
    Input::pollStart();
    PhysicsImpl::pollPhysics();
}

// poll the various program events
void Renderer::postFramePollEvents() {
    // update the input system
    flushRenderCommands(); // draw using the render commands and flush them after.
    Debug::DebugUI::get().drawImGuiInterface_framebegin();
    Debug::DebugUI::get().drawImGuiInterface_frameend();
    Entity::_transformFlagReset();
    Input::pollEnd();
}

// clean up the program after exiting
void Renderer::cleanup() {
    Logging::Logger::get().logInfo("Program recieved shutdown request, or was shutdown by the user. Attempting to clean up resources before closing...");
    // free any archives that were kept loaded for whatever reason
    ResourceManager::freeArchives();
    // destroy the imgui context
    Debug::DebugUI::get()._destroyImGuiContext();
    // destroy the gl context
    GLContext::get().destroyGLContext();
    // close the log file
    Logging::Logger::closeLog();
}

void Renderer::queueRenderCommand(uint8_t layer, uint64_t sortKey, IComponent* component, void(*draw_callback)(IComponent*)) {
    m_commandBuckets[layer].push_back({sortKey, component, draw_callback});
    m_bucketHasData[layer] = true;
}

void Renderer::flushRenderCommands() {
    // LayerRegistry::get() is annoying to type
    LayerRegistry& registry = LayerRegistry::get();

    // iterate through all our layers
    for (const auto& layer : registry.getLayerStack()) {
        // if there is no data for this layer, skip it.
        if (!m_bucketHasData[layer.m_layerID]) continue;

        // get the render command bucket
        auto& bucket = m_commandBuckets[layer.m_layerID];

        // sort the render commands
        std::sort(bucket.begin(), bucket.end(), [](const RenderCommand& a, const RenderCommand& b) {
            return a.m_sortKey < b.m_sortKey;
        });

        // store a ptr to the actual active cam
        auto activeCam = Camera::activeCamera();
        // store the active framebuffer
        auto camFB = Camera::activeCamera()->m_framebuffer;
        
        // to avoid any desync in something like portals or mirrors,
        // or whatever you may use framebuffers for, we render the
        // other cameras first, so their views are up-to-date.
        // then we render the actual camera view last.

        for (const auto& fb : Framebuffer::framebufferMap()) {
            if (!fb.second.lock()) continue;
            
            auto fbPtr = fb.second.lock();
            // the main camera view gets rendered last
            
            if (fbPtr == camFB) continue;
            
            // get this cam instance
            auto thisCam = fbPtr->getCamera();
            // this is for cameras, if the framebuffer is on something else,
            // the user has to sort that out
            if (!thisCam) continue;

            // no point rendering for a camera that isn't enabled
            if (!thisCam->getEntity().m_enabled) continue; 

            // set this as the active camera just to render.
            Camera::setActiveCamera(thisCam);
         
            // bind the framebuffer
            fbPtr->bindFramebuffer();

            // clear the screen
            glClear(thisCam->m_clearFlags);
            glClearColor(thisCam->m_clearCol.x, thisCam->m_clearCol.y, thisCam->m_clearCol.z, thisCam->m_clearCol.w);

            // run the draw functions 
            for (const auto& command : bucket) {
                command.drawCallback(command.m_component);
            }
            
        }

        // reset the active camera
        Camera::setActiveCamera(activeCam);
        camFB->bindFramebuffer();
        
        // clear the screen 
        glClearColor(activeCam->m_clearCol.x, activeCam->m_clearCol.y, activeCam->m_clearCol.z, activeCam->m_clearCol.w);
        glClear(activeCam->m_clearFlags);

        // run the draw functions once more 
        for (const auto& command : bucket) {
            command.drawCallback(command.m_component);
        }

        GLContext::get().glState.m_currentFramebuffer = 0;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        _drawView(camFB.get());

        // draw callbacks ran, we can clear the bucket now.
        bucket.clear(); 
        m_bucketHasData[layer.m_layerID] = false;

    }
}

uint64_t Renderer::genSortKey(bool transparent, Material *mat, float depth) {

    // materials have more than an albedo key, but albedo is the most common
    // and therefore the texture we should prioritise
    uint16_t albedoKey = (mat->m_data.m_hasAlbedoTx) ? 
        static_cast<uint16_t>(mat->getAlbedoTx()->getUnitID() & 0xFFFF) : 0;
    // get the shader program ID
    uint16_t programID = mat->m_program->getProgramID();

    // reduce the depth key down to a 32-bit int for bit shifting
    uint32_t depthKey = std::bit_cast<uint32_t>(depth);
    uint64_t key = 0;

    // ensure we prioritise the right values in different cases.
    // std::sort will eat this shit up
    if (!transparent) {
        // set up the key
        key |= (static_cast<uint64_t>(programID) << 48);    // shader program has priority
        key |= (static_cast<uint64_t>(albedoKey) << 32);    // second priority is the albedo texture
        key |= depthKey;                                    // Priority 3: Depth tie-breaker (32 bits)
        return key;
    }
    else {
        depthKey = ~depthKey; // invert the depth key (back-to-front sorting)
        key |= (static_cast<uint64_t>(depthKey) << 32);      // for transparency, depth has to take priority
        key |= (static_cast<uint64_t>(programID) << 16);     // then the shader program
        key |= albedoKey;                                    // finally the albedo texture
        return key;
    }
}

uint16_t Renderer::compressPtrToHash(const void* ptr) {
    uintptr_t ptrVal = reinterpret_cast<uintptr_t>(ptr);
    return static_cast<uint16_t>((ptrVal ^ (ptrVal >> 16) ^ (ptrVal >> 32)) & 0xFFFF);
}

void Renderer::_drawView(Framebuffer *fb) {

    glDisable(GL_DEPTH_TEST);
    // set the viewport size to fit the window
    glViewport(0, 0, m_wndSize.x, m_wndSize.y);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT);

    // set the material texture to the rendertexture
    m_mainFramebufferMaterial->setAlbedoTx(fb->m_renderTexture);

    // bind our stuff
    m_mainFramebufferMesh->bindMesh();
    
    // use this material for drawing
    m_mainFramebufferMaterial->useMaterial();

    // draw the stuff!
    glDrawElements(m_mainFramebufferMesh->getTopology(), m_mainFramebufferMesh->getIndexCount(), GL_UNSIGNED_SHORT, 0);

}