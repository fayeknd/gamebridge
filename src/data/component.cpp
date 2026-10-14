#include <component.hpp>
#include <entity.hpp>

using namespace Engine;

Entity& EntityComponent::getEntity() {
    return *m_entity;
}

const Entity& EntityComponent::getEntity() const {
    return *m_entity;
}

const ComponentTypeID EntityComponent::getTypeID() const {
    return m_typeID;
}