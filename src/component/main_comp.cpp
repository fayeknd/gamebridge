#include <cachable.hpp>
#include <logger.hpp>
#include <physics/ray.hpp>
#include <shader.hpp>
#include <component/collider.hpp>
#include <component/boundingbox.hpp>
#include <component/main_comp.hpp>
#include <entity.hpp>
#include <physics/physics.hpp>

#include <component/renderable.hpp>
#include <resources.hpp>
#include <renderer.hpp>
#include <component/camera.hpp>
#include <time.hpp>
#include <input.hpp>
#include <texture.hpp>
#include <utils/debug.hpp>
#include <physics/rigidbody.hpp>

using namespace Engine;
using namespace Physics;
using namespace Engine::Debug;

Entity* square;
Entity* square2; 
Entity* cam;
Entity* teenySquare;
Rigidbody* rb;
float sensitivity = 0.45;
fm::vec3 ray_start;
fm::vec3 ray_end;


void MainComponent::init() {

    Renderer::get().setWndSize({1280, 720});

    // create entity and add component
    square = Entity::create("Test Square").get(); 
    square2 = Entity::create("Test Square 2").get();
    
    rb = square->addComponent<Rigidbody>();
    auto box = square->addComponent<Collider>(ColliderShape::Box);
    auto renderer = square->addComponent<IRenderable>();
    rb->m_mass = 1000.f;
    rb->m_useGravity = true;
    


    auto box2 = square2->addComponent<Collider>(ColliderShape::Box);
    auto renderer2 = square2->addComponent<IRenderable>();


    square->transform.addPositionY(-10.0f);
    square2->transform.setScaleX(10.0f);
    square2->transform.setScaleZ(10.0f);
    square2->transform.addPositionY(20.0f);
    square2->transform.addScale(5.0f);

    // add some basic data to the renderer
    renderer->m_mesh = Mesh::primitive(Primitive::Cube);
    renderer->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer->m_material->m_data.m_albedoFac.w = 0.4f;
    renderer->m_material->m_data.m_hasTransparency = true;

    renderer2->m_mesh = Mesh::primitive(Primitive::Cube);
    renderer2->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer2->m_material->m_data.m_albedoFac.w = 0.4f;
    renderer2->m_material->m_data.m_hasTransparency = true;
    //renderer2->m_material->m_data.m_albedoFac.y = 0.4f;

    //square->setLayer(Layer::create("TestLayer", 1));
    //square->getLayer()->m_ignoreRaycast = true;

    // create the camera entity
    cam = Entity::create("Camera").get();
    // add a camera component
    auto camComponent = cam->addComponent<Camera>();

    // push the camera back a bit in the world
    cam->transform.addPositionZ(-5);
    // set the clear colour on the camera
    camComponent->m_clearCol = {0.1f, 0.1f, 0.1f, 1.0f};

    // set the target fps
    //Renderer::get().setVSyncMode(VSyncMode::On);
    Renderer::get().setTargetFPS(240);
    // free any archives.
    ResourceManager::freeArchives();
}

void MainComponent::update(double delta) {
    //square2->transform.addRotation(delta * 10.0f);

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

    if (Input::getActionDown(Key::F)) {
        RaycastHit out = Physics::Raycast({cam->transform.getGlobalPosition(), cam->transform.getGlobalFront()}, LayerMasks::All);

        ray_start = cam->transform.getGlobalPosition();
        ray_end = ray_start + (cam->transform.getGlobalFront() * out.m_hitInfo.m_distance);

        DBG.logInfo("Made a Raycast. Did it hit? {}", out.m_hitInfo.m_hasHit);
    }

    if (Input::getActionDown(Key::C)) {
        square->transform.addPositionY(-10.0f);
    }
}

void MainComponent::draw() {
    static DebugLine line;
    line.draw({
        Point({ray_start}),
        Point({ray_end})
    });
}

void MainComponent::lateUpdate(double delta) {

}

void MainComponent::physicsUpdate(double delta) {
    //rb->addForce(fm::vec3(0, -1, 0), Rigidbody::ForceMode::Impulse); // just testing
    //Logging::Logger::get().logInfo("Velocity: {}", rb->getVelocity());
}

void MainComponent::onDestroy() {

}