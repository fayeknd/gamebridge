#include <cachable.hpp>
#include <logger.hpp>
#include <ray.hpp>
#include <shader.hpp>
#include <component/collider.hpp>
#include <component/boundingbox.hpp>
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
using namespace Physics;

Entity* square;
Entity* cam;
Entity* teenySquare;
float sensitivity = 0.45;

void MainComponent::init() {

    Window::get().setWndSize({1280, 720});

    // create entity and add component
    square = Entity::create("Test Square").get(); 
    auto collider = square->addComponent<BoundingBox>();
    auto renderer = square->addComponent<IRenderable>();
    collider->m_renderBounds = false;
    collider->m_max += {0.1f};
    collider->m_min -= {0.1f};
    collider->collider = std::shared_ptr<Collider>(new Collider(ColliderShape::Sphere));

    square->transform.addPosition(1.1f);
    square->transform.addScaleY(.3f);

    // add some basic data to the renderer
    renderer->m_mesh = Mesh::primitive(Primitive::Cube);
    renderer->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer->m_material->m_data.m_albedoFac.y = 0.4f;


    // create the camera entity
    cam = Entity::create("Camera").get();
    // add a camera component
    auto camComponent = cam->addComponent<Camera>();

    // push the camera back a bit in the world
    cam->transform.addPositionZ(-5);
    // set the clear colour on the camera
    camComponent->m_clearCol = {0.1f, 0.1f, 0.1f, 0.7f};

    // set the target fps
    //Window::get().setVSyncMode(VSyncMode::On);
    Window::get().setTargetFPS(240);
    // free any archives.
    ResourceManager::freeArchives();
}

void MainComponent::update(double delta) {
    square->transform.setScaleX((sin(Time::sinceStartup()) / 10.0f) + 1.0f);

    if (Input::getActionDown(Key::F1)) {
        DebugUI::get().m_drawDebugger = !DebugUI::get().m_drawDebugger;
    }

    if (DebugUI::get().m_drawDebugger && Input::getAction(MouseBtn::RightMouse) || 
       (!DebugUI::get().m_drawDebugger)) {
        
        if (!DebugUI::get().getIO().WantCaptureMouse) Input::setMouseHidden(true);

        auto mdelta = Input::getMouseDelta();
        if (mdelta != fm::dvec2::zero()) {
            cam->transform.addRotationX(mdelta.y * sensitivity);
            cam->transform.addRotationY(mdelta.x * sensitivity);
            cam->transform.setRotationX(std::clamp(cam->transform.getRotationX(), -90.0f, 90.0f));
        }
    }
    else {
        Input::setMouseHidden(false);
    }
    // basic input handling test
    if (Input::getAction(Key::W)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getFront() * speed);
    }
    if (Input::getAction(Key::D)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getRight() * speed);
    }
    if (Input::getAction(Key::E)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getUp() * speed);
    }

    if (Input::getAction(Key::S)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getBack() * speed);
    }
    if (Input::getAction(Key::A)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getLeft() * speed);
    }
    if (Input::getAction(Key::Q)) {
        cam->transform.addPosition(static_cast<float>(1.0f * delta) * cam->transform.getDown() * speed);
    }

    static fm::vec3 ray_start;
    static fm::vec3 ray_end;

    if (Input::getActionDown(Key::F)) {
        RaycastHit out = Physics::Raycast({cam->transform.getGlobalPosition(), cam->transform.getGlobalFront()});

        ray_start = cam->transform.getGlobalPosition();
        ray_end = ray_start + (cam->transform.getGlobalFront() * out.m_hitInfo.m_distance);

        Logging::Logger::get().logInfo("Made a Raycast. Did it hit? {}", out.m_hitInfo.m_hasHit);
    }

    static DebugLine line;
    line.draw({
        Point({ray_start}),
        Point({ray_end})
    });
}

void MainComponent::lateUpdate(double delta) {

}

void MainComponent::onDestroy() {

}