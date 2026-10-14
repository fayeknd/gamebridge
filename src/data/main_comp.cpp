#include <main_comp.hpp>
#include <entity.hpp>

using namespace Engine;

void MainComponent::init() {

}

void MainComponent::update(float delta) {
    std::cout << "Running in update function of entity with name " << getEntity().m_name << std::endl;
}

void MainComponent::lateUpdate(float delta) {

}

void MainComponent::onDestroy() {

}