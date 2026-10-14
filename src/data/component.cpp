#include "component.hpp"
#include "entity.hpp"

using namespace Engine;

template <typename T>
ComponentTypeID EntityComponent::getComponentTypeID() {

    if (!std::is_base_of<EntityComponent, T>::value) {
        std::cerr << "Provided class is not a component!\n"; 
        return -1;
    }
    ComponentTypeID id = s_componentTypePool.getNewID();
    return id;

}