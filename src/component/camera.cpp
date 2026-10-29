#include "logger.hpp"
#include <component/camera.hpp>
#include <renderer.hpp>
#include <entity.hpp>
using namespace Engine;

// constructor
Camera::Camera() {
    // is camera queue empty?
    if (s_cameraQueue.size() == 0) setActiveCamera(this); // yes, no active camera yet. set this one.
}

void Camera::init() {
    // add this camera to the queue after 
    // the class is fully initialised.
    s_cameraQueue.push(weak_from_this());
    m_framebuffer = Framebuffer::create();
    m_framebuffer->m_camera = this;
}

// destructor
Camera::~Camera() {
    // if this is the active camera, we need a new one.
    if (s_activeCamera != this) return;
    
    
    // next camera reference
    Camera* nextCamera = nullptr;
    // while the next camera is null
    while (!nextCamera) {
        // if the camera queue is empty, 
        // we will just be in an infinite loop.
        if (s_cameraQueue.size() == 0) break;
        // if the front pointer is invalid, remove it.
        auto front = s_cameraQueue.front(); 
        if (front.expired() || front.lock() == shared_from_this()) {
            s_cameraQueue.pop();
            // skip the rest of the loop
            continue;
        } 
        else {
            // we have the next valid camera in the queue
            nextCamera = front.lock().get();
            setActiveCamera(nextCamera);
        };
    }
    
}

void Camera::setActiveCamera(Camera *c) {
    if (!c) return;
    if (s_activeCamera) {
        // disable all the old functions
        for (size_t i = 0; i < s_activeCamera->m_glFunctions.size(); i++) {
            glDisable(s_activeCamera->m_glFunctions[i]);
        }
    }
    s_activeCamera = c;
    // enable the new functions
    for (size_t i = 0; i < s_activeCamera->m_glFunctions.size(); i++) {
        glEnable(s_activeCamera->m_glFunctions[i]);
    }
}

// get the active FREAKINGGG CAMERRAAAAA!!!!
Camera* Camera::activeCamera() {
    return s_activeCamera; 
}

// set the projection mode and mark projectionDirty as true        
void Camera::setProjectionMode(ProjectionMode pm) {
    m_projMode = pm;
    m_projectionDirty = true;
}
// get the projection mode
ProjectionMode Camera::getProjectionMode() {
    return m_projMode; 
}

fm::mat4 Camera::getProjectionMat() {
    // return if the projection matrix isn't dirty 
    // calculate anyway if the viewport changed
    if (!m_projectionDirty && !Renderer::get().m_viewportDirty ||  m_lastCalculatedFrame_p == Renderer::get().getFrame()) return m_projectionMatrix;
    m_projectionDirty = false;

     m_lastCalculatedFrame_p = Renderer::get().getFrame();

    // switch projection mode cases
    switch (m_projMode) {
        case ProjectionMode::Orthographic: {

            float height = m_orthoSize;
            float width = height * Renderer::get().getAspectRatio();

            // make the projection matrix
            m_projectionMatrix = fm::orthographic<float>(
                -width,
                 width,
                -height,
                 height, 
                m_nearClip, m_farClip);
            break;
            }
        default: // ProjectionMode::Perspective
            m_projectionMatrix = fm::perspective<float>(
                m_fov,
                Renderer::get().getAspectRatio(),
                m_nearClip,
                m_farClip);
            break;
    }
    return m_projectionMatrix;
}
fm::mat4 Camera::getViewMat() {
    // return if the transform was not touched this frame
    if (!entity.wasTransformDirty() || m_lastCalculatedFrame_v == Renderer::get().getFrame()) return m_viewMatrix;
    entity.m_queueTransformReset = true;
    // (pos, pos + front, up)

    m_lastCalculatedFrame_v = Renderer::get().getFrame();
    
    m_viewMatrix = fm::lookAt(
        entity.transform.getGlobalPosition(),
        entity.transform.getGlobalPosition() + entity.transform.getGlobalFront(),
        entity.transform.getGlobalUp());
    return m_viewMatrix;
}

float Camera::getFOV() {
    return m_fov;
}
float Camera::getNearClip() {
    return m_nearClip;
} 
float Camera::getFarClip() {
    return m_farClip;
}
float Camera::getOrthoSize() {
    return m_orthoSize;
}

void Camera::setFOV(float v) {
    m_fov = v;
    m_projectionDirty = true;
}
void Camera::setNearClip(float v) {
    m_nearClip = v;
    m_projectionDirty = true;
}
void Camera::setFarClip(float v) {
    m_farClip = v;
    m_projectionDirty = true;
}
void Camera::setOrthoSize(float v) {
    m_orthoSize = v;
    m_projectionDirty = true;
}