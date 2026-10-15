#include <component/main_comp.hpp>
#include <entity.hpp>

#include <component/renderable.hpp>
#include <resources.hpp>

using namespace Engine;

void MainComponent::init() {
    
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

    square->transform.setPositionX(0.5f);

    glClearColor(1, 0.5, 1, 1);
}

void MainComponent::update(float delta) {

}

void MainComponent::lateUpdate(float delta) {

}

void MainComponent::onDestroy() {

}