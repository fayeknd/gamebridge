#include <component/player.hpp>
#include <input.hpp>
#include <entity.hpp>
#include <utils/debug.hpp>
#include <component/collider.hpp>
#include <physics/rigidbody.hpp>
#include <logger.hpp>

using namespace Engine;
using namespace Physics;
using namespace Engine::Debug;

void Player::init() {
    camObject = Entity::create("Camera").get();
    camObject->setParent(&entity);
    rb = entity.addComponent<Rigidbody>().get();

    rb->m_mass = 1000000000000000000.f;
    rb->m_useGravity = true;

    collider = entity.addComponent<Collider>(ColliderShape::Capsule).get();
    cam = camObject->addComponent<Camera>().get();
    camObject->transform.addPositionY(collider->m_height / -2.f);
    cam->m_clearCol = {0.1f, 0.1f, 0.1f, 1.0f};

    toggleMouseMovement = true;
    toggleMovement = true;
}

void Player::lateUpdate(double delta) {
    mouseInput(delta);
}

void Player::update(double delta) {

}

void Player::physicsUpdate(double delta) {
    movementInput(delta);
    //camObject->transform.setGlobalPosition(getEntity().transform.getPositionX(), getEntity().transform.getPositionY() + collider->m_height / 2.f, getEntity().transform.getPositionZ());
}

void Player::onDestroy() {
    
}

void Player::draw() {
    
}

void Player::movementInput(double delta) {

    if (!toggleMovement) return;

    // actual player front
    fm::vec3 front = camObject->transform.getRight().cross(fm::vec3::yAxis());
    // movement direction this frame (normalized)
    fm::vec3 mov = fm::vec3::zero();

    //WASD Movement
    if (Input::getAction(Key::W)) {
        mov += front;
    }
    if (Input::getAction(Key::A)) {
        mov += camObject->transform.getLeft();
    }
    if (Input::getAction(Key::S)) {
        mov += -front;
    }
    if (Input::getAction(Key::D)) {
        mov += camObject->transform.getRight();
    }
    // add the normalized movement vector to the position, multiplied by speed
    mov = static_cast<float>(delta) * mov.normalized() * speed;
    // NaN check. if a number is NaN, num == num will be false. 
    if (mov == mov) getEntity().transform.addPosition(mov);
}

void Player::mouseInput(double delta) {
    if (toggleMouseMovement) {
        Input::setMouseHidden(true);
        auto mdelta = Input::getMouseDelta();
        if (mdelta != fm::dvec2::zero()) {
            camObject->transform.addRotationX(mdelta.y * sensitivity);
            camObject->transform.addRotationY(mdelta.x * sensitivity);
            camObject->transform.setRotationX(std::clamp(camObject->transform.getRotationX(), -90.0f, 90.0f));
        }
    }
}