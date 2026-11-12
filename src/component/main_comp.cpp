#include <cachable.hpp>
#include <logger.hpp>
#include <physics/ray.hpp>
#include <shader.hpp>
#include <component/collider.hpp>
#include <physics/boundingbox.hpp>
#include <component/main_comp.hpp>
#include <entity.hpp>
#include <physics/physics.hpp>

#include <component/modeldraw.hpp>
#include <resources.hpp>
#include <renderer.hpp>
#include <component/camera.hpp>
#include <time.hpp>
#include <input.hpp>
#include <texture.hpp>
#include <utils/debug.hpp>
#include <physics/rigidbody.hpp>
#include <component/player.hpp>
#include <utils/debugcamera.hpp>

using namespace Engine;
using namespace Physics;
using namespace Engine::Debug;

Entity* square;
Entity* square2; 
Entity* player;
Player* playerComponent;
Entity* square3;
Rigidbody* rb;
float sensitivity = 0.45;
fm::vec3 ray_start;
fm::vec3 ray_end;


void MainComponent::init() {

    Renderer::get().setWndSize({1280, 720});

    // create entity and add component
    square = Entity::create("Test Square").get(); 
    square2 = Entity::create("Test Square 2").get();
    square3 = Entity::create("Test Square 3").get();
    player = Entity::create("Player").get();

    rb = square->addComponent<Rigidbody>().get();
    auto box = square->addComponent<Collider>(ColliderShape::Cylinder);
    auto renderer = square->addComponent<ModelDraw>();
    rb->m_mass = 1000.f;
    rb->m_useGravity = true;

    square2->transform.setScaleX(10.0f);
    square2->transform.setScaleZ(10.0f);
    square2->transform.addPositionY(20.0f);
    square2->transform.addScale(5.0f);

    Renderer::get().setWndTransparent(false);

    auto box2 = square2->addComponent<Collider>(ColliderShape::Box);
    auto renderer2 = square2->addComponent<ModelDraw>();

    auto rb2 = square3->addComponent<Rigidbody>();
    auto box3 = square3->addComponent<Collider>(ColliderShape::Box);
    auto renderer3 = square3->addComponent<ModelDraw>();
    rb2->m_mass = 1000.f;
    rb2->m_useGravity = true;

    playerComponent = player->addComponent<Player>().get();


    square->transform.addPositionY(-10.0f);
    square3->transform.addPositionY(-20.0f);


    // add some basic data to the renderer
    renderer->m_model = Model::primitive(Primitive::Cube);
    renderer->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer->m_material->m_data.m_albedoFac.w = 0.4f;
    renderer->m_material->m_data.m_hasTransparency = true;

    renderer2->m_model = Model::primitive(Primitive::Cube);
    renderer2->m_material->setAlbedoTx(Texture2D::createTextureFromPath("stuff=>textures/me trying to flirt.png"));
    renderer2->m_material->m_data.m_albedoFac.w = 0.4f;
    renderer2->m_material->m_data.m_hasTransparency = true;

    renderer3->m_model = Model::createModelFromPath("stuff=>models/bottle.fbx", &renderer3->m_materials);
    //renderer2->m_material->m_data.m_albedoFac.y = 0.4f;

    //square->setLayer(Layer::create("TestLayer", 1));
    //square->getLayer()->m_ignoreRaycast = true;

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
        playerComponent->toggleMouseMovement = !DebugUI::get().m_drawDebugger;
        playerComponent->toggleMovement = !DebugUI::get().m_drawDebugger;
        
        static EntityID debugCamID;

        if (DebugUI::get().m_drawDebugger) {
            Entity* debugCam = Entity::create("Debug Camera").get();
            debugCam->transform.setGlobalPosition(playerComponent->camObject->transform.getGlobalPosition());
            DebugCam* debugCamComponent = debugCam->addComponent<DebugCam>().get();
            debugCamID = debugCam->getID();
            Camera::setActiveCamera(debugCamComponent->cam);
        }
        else {
            Camera::setActiveCamera(playerComponent->cam);
            Entity::getEntityViaID(debugCamID)->release();
        }

    }


    // test
    if (Input::getActionDown(Key::F)) {
        RaycastHit out = Physics::Raycast({Camera::activeCamera()->getEntity().transform.getGlobalPosition(), Camera::activeCamera()->getEntity().transform.getGlobalFront()}, LayerMasks::All);

        ray_start = cam->transform.getGlobalPosition();
        ray_end = ray_start + (cam->transform.getGlobalFront() * out.m_hitInfo.m_distance);

        DBG.logInfo("Made a Raycast. Did it hit? {}", out.m_hitInfo.m_hasHit);
    }

    if (Input::getActionDown(Key::C)) {
        square->transform.setPositionY(-10.0f);
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