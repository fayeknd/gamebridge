#include <component/player.hpp>
#include <input.hpp>
#include <entity.hpp>
#include <utils/debug.hpp>
#include <component/collider.hpp>
#include <physics/rigidbody.hpp>

using namespace Engine;
using namespace Physics;
using namespace Engine::Debug;

void Player::init() {
    camObject = Entity::create("Camera").get();
    camObject->setParent(&entity);
    rb = entity.addComponent<Rigidbody>();

    rb->m_mass = 1000000000000000000.f;
    rb->m_useGravity = true;

    collider = entity.addComponent<Collider>(ColliderShape::Capsule);
    cam = camObject->addComponent<Camera>();
    camObject->transform.addPositionY(collider->m_height / -2.f);
    cam->m_clearCol = {0.1f, 0.1f, 0.1f, 1.0f};

    toggleMouseMovement = true;
    toggleMovement = true;
    Input::setMouseHidden(false);
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

    if (toggleMovement)
    {
        //WASD Movement
        if (Input::getAction(Key::W)) {
            getEntity().transform.addPosition(static_cast<float>(delta) * fm::vec3(camObject->transform.getFront().x, 0 , camObject->transform.getFront().z) * speed);
        }
        if (Input::getAction(Key::A)) {
            getEntity().transform.addPosition(static_cast<float>(delta) * fm::vec3(camObject->transform.getLeft().x, 0 , camObject->transform.getLeft().z) * speed);
        }
        if (Input::getAction(Key::S)) {
            getEntity().transform.addPosition(static_cast<float>(delta) * fm::vec3(camObject->transform.getBack().x, 0 , camObject->transform.getBack().z) * speed);
        }
        if (Input::getAction(Key::D)) {
            getEntity().transform.addPosition(static_cast<float>(delta) * fm::vec3(camObject->transform.getRight().x, 0 , camObject->transform.getRight().z) * speed);
        }
    }
}

void Player::mouseInput(double delta) {
    if (toggleMouseMovement) {
        auto mdelta = Input::getMouseDelta();
        if (mdelta != fm::dvec2::zero()) {
            camObject->transform.addRotationX(mdelta.y * sensitivity);
            camObject->transform.addRotationY(mdelta.x * sensitivity);
            camObject->transform.setRotationX(std::clamp(camObject->transform.getRotationX(), -90.0f, 90.0f));
        }
    }
}