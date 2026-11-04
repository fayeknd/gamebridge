#include <utils/debugcamera.hpp>
#include <utils/debug.hpp>
#include <component/component.hpp>
#include <input.hpp>
#include <entity.hpp>

using namespace Engine;
using namespace Engine::Debug;

void DebugCam::init() {
    cam = getEntity().addComponent<Camera>();
    DebugUI::get().m_drawDebugger = !DebugUI::get().m_drawDebugger;
}

void DebugCam::lateUpdate(double delta) {
    
}

void DebugCam::update(double delta) {
    if (DebugUI::get().m_drawDebugger && Input::getAction(MouseBtn::RightMouse)) {
        if (!DebugUI::get().getIO().WantCaptureMouse) Input::setMouseHidden(true);
        mouseInput(delta);
    }
    else {
        Input::setMouseHidden(false);
    }
}

void DebugCam::physicsUpdate(double delta) {
    movementInput(delta);
}

void DebugCam::onDestroy() {
    
}

void DebugCam::draw() {
    
}

void DebugCam::movementInput(double delta) {

    //WASD Movement
    if (Input::getAction(Key::W)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getFront() * cameraSpeed);
    }
    if (Input::getAction(Key::A)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getLeft() * cameraSpeed);
    }
    if (Input::getAction(Key::S)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getBack() * cameraSpeed);
    }
    if (Input::getAction(Key::D)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getRight() * cameraSpeed);
    }
    if (Input::getAction(Key::E)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getUp() * cameraSpeed);
    }
    if (Input::getAction(Key::Q)) {
        getEntity().transform.addPosition(static_cast<float>(delta) * getEntity().transform.getDown() * cameraSpeed);
    }
}

void DebugCam::mouseInput(double delta) {
    auto mdelta = Input::getMouseDelta();
    if (mdelta != fm::dvec2::zero()) {
        getEntity().transform.addRotationX(mdelta.y * sensitivity);
        getEntity().transform.addRotationY(mdelta.x * sensitivity);
        getEntity().transform.setRotationX(std::clamp(getEntity().transform.getRotationX(), -90.0f, 90.0f));
    }
}