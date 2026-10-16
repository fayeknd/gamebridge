#include <component/main_comp.hpp>
#include <entity.hpp>

#include <component/renderable.hpp>
#include <resources.hpp>
#include <window.hpp>

using namespace Engine;

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
    auto square = Entity::create("Test Square"); 
    auto renderer = square->addComponent<IRenderable>();

    renderer->m_mesh = mesh;
    renderer->m_shaderProgram = program;

    square->transform.setRotationX(70);

    glClearColor(1, 0.5, 1, 1);
}

void MainComponent::update(double delta) {
    std::cout << "The frame delta is " << delta << ". Target FPS: " << Window::get().getTargetFPS() << ", Current FPS: " << static_cast<double>(1) / delta << std::endl;
}

void MainComponent::lateUpdate(double delta) {

}

void MainComponent::onDestroy() {

}