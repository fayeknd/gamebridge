#pragma once
#include <gl_context.hpp>
#include <texture.hpp>
#include <logger.hpp>
#include <cachable.hpp>
#include <idpool.hpp>

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

        // the pointer to the camera this framebuffer MAY be on
        Camera* m_camera = nullptr;

        // our framebuffer id
        FramebufferID m_framebufferID;

        // framebuffer id
        GLuint m_bufferID;

        // renderbuffer id
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

        friend class Camera;

    public:

        // the framebuffer map. a weak_ptr so it does not contribute to the ref count.
        static inline std::unordered_map<FramebufferID, std::weak_ptr<Framebuffer>> framebufferMap() {
            static std::unordered_map<FramebufferID, std::weak_ptr<Framebuffer>> instance;
            return instance;
        }

        // create a new framebuffer instance
        static std::shared_ptr<Framebuffer> create(fm::uvec2 size = fm::uvec2::zero());

        ~Framebuffer();

        // use this framebuffer
        void bindFramebuffer();

        // the render texture
        std::shared_ptr<Texture2D> m_renderTexture;

        // get the framebuffer size
        const fm::uvec2 getSize() const { return m_size; }

        // set the size
        void setSize(fm::uvec2 size);

        // does this framebuffer resize with the window?
        const bool doesResizeWithWnd() const { return m_resizeWithWnd; }

        // set the value of m_resizeWithWnd
        void setResizeWithWnd(bool v); 

        // get our framebuffer cache id 
        const FramebufferID getFramebufferMapID() const { return m_framebufferID; }
        // get our framebuffer id 
        const FramebufferID getFramebufferID() const { return m_bufferID; }


        Camera* getCamera() { return m_camera; }
        
    };
}