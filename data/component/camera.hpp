#pragma once
#include <component/component.hpp>
#include <fm/fmath.hpp>
#include <gl_context.hpp>
#include <queue>
#include <framebuffer.hpp>

namespace Engine {

    using CameraID = uint8_t; // if there are more than 255 cameras we are doing something wrong
    
    // the camera projection mode
    enum ProjectionMode {

        Perspective     = 0x00,
    
        Orthographic    = 0x01
    
    };

    // finally! camera stuff!

    class Camera : public IComponent, public std::enable_shared_from_this<Camera> {
    protected:

        // camera's "model" matrix
        fm::mat4 m_viewMatrix;

        // projection matrix
        fm::mat4 m_projectionMatrix;

        // the camera fov (wouldve been a uint8 but you
        // MIGHT want a fractional value on this maybe?)
        float m_fov = 75;
        // the near frustum clip
        float m_nearClip = 0.01f;
        // the far frustum clip
        float m_farClip = 1000.0f;

        // orthographic or perspective?
        ProjectionMode m_projMode = ProjectionMode::Perspective;
        
        int m_lastCalculatedFrame_v = -1;
        int m_lastCalculatedFrame_p = -1;

        // for the lazy calculations
        bool m_projectionDirty = true;
        // we would have an m_viewDirty 
        // but that is actually the same as the entity
        // m_transformDirty boolean. There may be multiple 
        // cameras on the same entity and setting this
        // would mess with their matrices, so we can't do that.
        
        // the active rendering camera
        static inline Camera* s_activeCamera = nullptr;

        // the orthographic size
        float m_orthoSize = 1.0f;

        // to ensure if the active camera is destroyed, 
        // we can set a new one.
        static inline std::queue<std::weak_ptr<Camera>> s_cameraQueue;

    component_public:
        static void setActiveCamera(Camera* c);
        static Camera* activeCamera();

        // register this as the main camera
        // if there is none
        Camera();

        // if this is the main camera, 
        // unregister it.
        ~Camera();

        std::shared_ptr<Framebuffer> m_framebuffer;

        // gl clear colour
        PUBLIC fm::col m_clearCol = fm::col(0, 0, 0, 1);
        // flags to clear at the end of a frame
        PUBLIC bitfield m_clearFlags  = (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        PUBLIC std::vector<GLuint> m_glFunctions = {GL_DEPTH_TEST, GL_CULL_FACE, GL_BLEND}; 

        // set the projection mode and mark projectionDirty as true        
        void setProjectionMode(ProjectionMode pm);
        // get the projection mode
        ProjectionMode getProjectionMode();

        fm::mat4 getProjectionMat();
        fm::mat4 getViewMat();
        
        // self-explanitory
        float getFOV();
        float getNearClip(); 
        float getFarClip();
        float getOrthoSize();

        void setFOV(float v);
        void setNearClip(float v);
        void setFarClip(float v);
        void setOrthoSize(float v);

        void init() override;

    };

};