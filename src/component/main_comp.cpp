#include <component/main_comp.hpp>
#include <entity.hpp>

#include <component/renderable.hpp>
#include <resources.hpp>
#include <window.hpp>
#include <component/camera.hpp>
#include <time.hpp>

using namespace Engine;

Entity* square;
Entity* cam;

void MainComponent::init() {

    Window::get().setWndSize({1280, 720});

    auto v = std::vector<Vertex>() = {
        {{ 0.5f,  0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}},
        {{ 0.7f, -0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}},
        {{-0.7f, -0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}},
        {{-0.5f,  0.5f, 0.0f}, {1.0,1.0,1.0,1.0}, {0.0,0.0}, {0.0,0.0,0.0}}
    };
    auto i = std::vector<unsigned short>() = {0, 1, 2, 2, 3, 0};

    auto mesh = Mesh::createMeshWithData(v, i);
    auto program = ShaderProgram::createProgramFromPath("resources=>shaders/shader.glsl");
    ResourceManager::freeArchives();

    // create entity and add component
    square = Entity::create("Test Square").get(); 
    auto renderer = square->addComponent<IRenderable>();

    // add some basic data to the renderer
    renderer->m_mesh = mesh;
    renderer->m_shaderProgram = program;

    // create the camera entity
    cam = Entity::create("Camera").get();
    // add a camera component
    auto camComponent = cam->addComponent<Camera>();

    // push the camera back a bit in the world
    cam->transform.addPositionZ(-5);
    // set the clear colour on the camera
    camComponent->m_clearCol = {1, 0, 1, 0.1f};

    // set the target fps
    Window::get().setTargetFPS(240);
}

void MainComponent::update(double delta) {
    square->transform.addRotationZ(10 * delta);

    // basic input handling test
    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_W) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getFront());
    }
    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_D) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getRight());
    }
    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_E) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getUp());
    }

    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_S) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getBack());
    }
    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_A) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getLeft());
    }
    if (glfwGetKey(Window::get().getWndHandle(), GLFW_KEY_Q) == GLFW_PRESS) {
        cam->transform.addPosition(static_cast<float>(1 * delta) * cam->transform.getDown());
    }
}

void MainComponent::lateUpdate(double delta) {

}

void MainComponent::onDestroy() {

}