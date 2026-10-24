#include "cachable.hpp"
#include "gl_context.hpp"
#include "logger.hpp"
#include "ray.hpp"
#include "shader.hpp"
#include <component/collider.hpp>
#include <component/main_comp.hpp>
#include <entity.hpp>
#include <physics.hpp>

#include <component/renderable.hpp>
#include <resources.hpp>
#include <window.hpp>
#include <component/camera.hpp>
#include <time.hpp>
#include <input.hpp>
#include <texture.hpp>

using namespace Engine;

void MainComponent::init() {

    Window::get().setWndSize({1280, 720});

    // create entity and add component
    square = Entity::create("Test Square").get(); 
    auto collider = square->addComponent<BoundingBox>();
    auto renderer = square->addComponent<IRenderable>();
    collider->m_renderBounds = true;
    
    collider->m_max += {0.1f};
    collider->m_min -= {0.1f};

    square->transform.addPosition(1.1f);
    square->transform.addScaleY(.3f);

    // TODO : implement copying entities
    teenySquare = Entity::create("hitSquare").get();
    auto renderer2 = teenySquare->addComponent<IRenderable>();

    // add some basic data to the renderer
    renderer->m_mesh = Mesh::primitive(Primitive::Cube);
    renderer->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer->m_material->m_data.m_albedoFac.y = 0.4f;

    renderer2->m_mesh = Mesh::primitive(Primitive::Cube);
    renderer2->m_material->m_data.m_albedoFac = {0.0f, 1.0f, 0.0f, 1.0f};

    teenySquare->transform.setScale(0.1f);
    // create the camera entity
    cam = Entity::create("Camera").get();
    // add a camera component
    auto camComponent = cam->addComponent<Camera>();

    // push the camera back a bit in the world
    cam->transform.addPositionZ(-5);
    // set the clear colour on the camera
    camComponent->m_clearCol = {0.1f, 0.1f, 0.1f, 0.0f};



    // set the target fps
    Window::get().setTargetFPS(240);
    // free any archives.
    ResourceManager::freeArchives();
}

void MainComponent::update(double delta) {
    square->transform.setScaleX((sin(Time::sinceStartup()) / 10.0f) + 1.0f);

    auto mdelta = Input::getMouseDelta();
    if (mdelta != fm::dvec2::zero()) {
        cam->transform.addRotationX(mdelta.y * sensitivity);
        cam->transform.addRotationY(mdelta.x * sensitivity);
        cam->transform.setRotationX(std::clamp(cam->transform.getRotationX(), -90.0f, 90.0f));
    }

    // basic input handling test
    if (Input::getAction(Key::W)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getFront());
    }
    if (Input::getAction(Key::D)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getRight());
    }
    if (Input::getAction(Key::E)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getUp());
    }

    if (Input::getAction(Key::S)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getBack());
    }
    if (Input::getAction(Key::A)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getLeft());
    }
    if (Input::getAction(Key::Q)) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getDown());
    }
    if (Input::getActionDown(Key::F)) {
        // then idk good job
    }


    RaycastHit out;    
    if(Physics::Raycast({cam->transform.getPosition(), cam->transform.getFront()}, out)) {
        teenySquare->forceSetEnabled(true);
        teenySquare->transform.setPosition(out.m_hitInfo.m_hitPoint);
    }
    else {
        teenySquare->setEnabled(false);
    }
}

void MainComponent::lateUpdate(double delta) {

}

void MainComponent::onDestroy() {

}