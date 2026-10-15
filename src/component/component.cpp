#include <component/component.hpp>
#include <entity.hpp>

using namespace Engine;

ComponentTypeID IComponent::_nextID() {
    static ComponentTypeID ctr = 0;
    return ctr++;
}

Entity& IComponent::getEntity() {
    return *m_entity;
}

const Entity& IComponent::getEntity() const {
    return *m_entity;
}

const ComponentTypeID IComponent::getTypeID() const {
    return m_typeID;
}