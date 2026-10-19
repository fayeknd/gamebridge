#include <component/camera.hpp>
#include <window.hpp>
#include <entity.hpp>
using namespace Engine;

// constructor
Camera::Camera() {
    // is camera queue empty?
    if (s_cameraQueue.size() == 0) s_activeCamera = this; // yes, no active camera yet. set this one.
}

void Camera::init() {
    // add this camera to the queue after 
    // the class is fully initialised.
    s_cameraQueue.push(weak_from_this());
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
            s_activeCamera = nextCamera;
        };
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
    if (!m_projectionDirty && !Window::get().m_viewportDirty) return m_projectionMatrix;
    m_projectionDirty = false;

    // switch projection mode cases
    switch (m_projMode) {
        case ProjectionMode::Orthographic: {
            // get the half aspect.
            // used for the orthographic bounds
            fm::uvec2 halfAspect = Window::get().getFractionalAspectRatio();
            halfAspect /= 2;

            // make the projection matrix
            m_projectionMatrix = fm::orthographic<float>(
                -halfAspect.x,
                 halfAspect.x,
                -halfAspect.y,
                 halfAspect.y, 
                m_nearClip, m_farClip);
            break;
            }
        default: // ProjectionMode::Perspective
            m_projectionMatrix = fm::perspective<float>(
                m_fov,
                Window::get().getAspectRatio(),
                m_nearClip,
                m_farClip);
            break;
    }
    return m_projectionMatrix;
}
fm::mat4 Camera::getViewMat() {
    // return if the transform was not touched this frame
    if (!entity.wasTransformDirty()) return m_viewMatrix;
    entity.m_queueTransformReset = true;

    // (pos, pos + front, up)
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
