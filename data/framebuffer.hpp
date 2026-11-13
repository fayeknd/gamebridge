#pragma once
#include <gl_context.hpp>
#include <texture.hpp>
#include <logger.hpp>
#include <cachable.hpp>
#include <idpool.hpp>
#include <material.hpp>

namespace Engine {

    class Camera;

    using FramebufferID = uint32_t;

    // the framebuffer renders a view (typically from a
    // camera) onto a render texture that can be applied 
    // to a surface.
    class Framebuffer : public ICachable<Framebuffer> {
    protected:

        // the framebuffer ID pool
        static inline IDPool<FramebufferID>& framebufferIDPool() {
            static IDPool<FramebufferID> instance;
            return instance;
        }

        // the pointer to the camera this framebuffer MAY be on.
        // doesn't need to be a smart pointer, if this isn't null,
        // then the camera could only exist as long as the framebuffer
        // does.
        // unless you're doing something VERY silly.
        Camera* m_camera = nullptr;

        // our framebuffer id
        FramebufferID m_fbMapID;

        // primary framebuffer id
        FramebufferID m_bufferID;

        // primary renderbuffer id
        GLuint m_renderBufferID;

        // generate the buffer
        void _genBuffer(fm::uvec2 size = fm::uvec2::zero());

        // delete the buffer
        void _deleteBuffer();

        // has this framebuffer been initialised yet?
        bool m_initialised = false;
        
        // should this framebuffer resize with the window?
        bool m_resizeWithWnd = false;

        static Logging::Logger& log() {
            static Logging::Logger instance("FRAMEBUFFER");
            return instance;
        }

        // the same as the render texture size.
        fm::uvec2 m_size;

        // framebuffer format
        GLuint m_format = GL_DEPTH24_STENCIL8;

        // the primary render texture
        std::shared_ptr<Texture2D> m_pingRenderTexture;

        // the secondary render texture
        std::shared_ptr<Texture2D> m_pongRenderTexture;

        // 0 for ping, 1 for pong
        bool m_pingPongState = 0;

        // set a specific render texture's size
        void _setSpecificTexSize(fm::uvec2 size, std::shared_ptr<Texture2D> tex);

        // run individual post process step
        void _doPostProcessImpl(std::shared_ptr<ShaderProgram> postprocess, std::shared_ptr<Texture2D> tex);

        friend class Camera;

    public:

        // postprocess material
        std::shared_ptr<Material> m_postProcessMaterial = nullptr;

        // the postprocess steps
        std::vector<std::shared_ptr<ShaderProgram>> m_postProcessSteps;

        // apply post processing effects to this framebuffer
        void doPostProcess();

        // the framebuffer map. a weak_ptr so it does not contribute to the ref count.
        static std::unordered_map<FramebufferID, std::weak_ptr<Framebuffer>>& framebufferMap() {
            static std::unordered_map<FramebufferID, std::weak_ptr<Framebuffer>> instance;
            return instance;
        }

        // create a new framebuffer instance
        static std::shared_ptr<Framebuffer> create(fm::uvec2 size = fm::uvec2::zero(), GLuint format = GL_DEPTH24_STENCIL8);

        ~Framebuffer();

        // use this framebuffer
        void bindFramebuffer(bool pong = false);

        // unbind any currently bound framebuffers 
        static void unbindFramebuffers();

        // get the framebuffer size
        const fm::uvec2 getSize() const { return m_size; }

        // set the size. returns true if it actually did something.
        bool setSize(fm::uvec2 size);

        // does this framebuffer resize with the window?
        const bool doesResizeWithWnd() const { return m_resizeWithWnd; }

        // set the value of m_resizeWithWnd
        void setResizeWithWnd(bool v); 

        // get our framebuffer cache id 
        const FramebufferID getFramebufferMapID()  const { return m_fbMapID; }
        // get our framebuffer id 
        const FramebufferID getFramebufferID()     const { return m_bufferID; }

        std::shared_ptr<Texture2D> getRenderTexture();

        // return a raw ptr to the camera.
        Camera* getCamera() { return m_camera; }
        
    };
}