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
    if (!m_render || !Camera::activeCamera() || !entity.isRenderable()) return;

    bool hasTransparency = false; // true if any material has transparency
    DepthType dt;
    if (m_materials.size() <= 0) dt = DepthType::Planar;
    else {
        
        // check if any of the materials have transparency.
        // this will not be any more expensive with one material.
    

        // loop through the materials
        for (const auto& mat : m_materials) {
        
            // does this material have transparency?
            if (mat->m_data.m_hasTransparency) {
                hasTransparency = true;
                break;
            }
        }
        dt = (hasTransparency) ? DepthType::SquaredRadial : DepthType::Planar;
    }
    // get the depth from the camera
    float depth = entity.transform.getDepth(
        Camera::activeCamera()->getEntity().transform.getGlobalPosition(),
        Camera::activeCamera()->getEntity().transform.getGlobalFront(),
        dt
    );

    Renderer& renderer = Renderer::get();

    // create the sort key
    uint64_t sortKey;
    if (m_materials.size() <= 0) sortKey = 1000; // render it above everything else
    else                         sortKey = renderer.genSortKey(hasTransparency, m_materials[0].get(), depth); // pass the first material, objects are most likely to mainly use the first material
    renderer.queueRenderCommand(layer->m_layerID, sortKey, this, _distributeDraw);
}

void IComponent::physicsUpdateImpl(double delta) {
    // do our physics update
    physicsUpdate(delta);
    if (entity.transform.isDirty()) {
        // we did do physics this frame.
        entity.m_didPhysicsThisTick = true;
    }
}