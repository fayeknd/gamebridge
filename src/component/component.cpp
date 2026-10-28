#include <component/component.hpp>
#include <entity.hpp>
#include <component/camera.hpp>
#include <renderer.hpp>

using namespace Engine;

bool IComponent::hasInitRun() {
    return m_hasInit;
}

// get the next available ID for a component type.
ComponentTypeID IComponent::_nextID() {
    // create static variable (once)
    static ComponentTypeID ctr = 0;
    // increment
    return ctr++;
}

// get the entity this component is assigned to
Entity& IComponent::getEntity() {
    return *m_entity;
}

// get the entity this component is assigned to
const Entity& IComponent::getEntity() const {
    return *m_entity;
}

// get the type id
const ComponentTypeID IComponent::getTypeID() const {
    return m_typeID;
}

void IComponent::drawImpl(const Layer *layer) {
    if (!m_material || !m_material->m_program || !m_render || !Camera::activeCamera()) return;

    // get the depth from the camera
    float depth = entity.transform.getDepth(
        Camera::activeCamera()->getEntity().transform.getGlobalPosition(),
        Camera::activeCamera()->getEntity().transform.getGlobalFront(),
        (m_material->m_data.m_hasTransparency) ? DepthType::SquaredRadial : DepthType::Planar
    );

    Renderer& renderer = Renderer::get();

    // create the sort key
    uint64_t sortKey = renderer.genSortKey(m_material->m_data.m_hasTransparency, m_material.get(), depth);
    renderer.queueRenderCommand(layer->m_layerID, sortKey, this, _distributeDraw);
}