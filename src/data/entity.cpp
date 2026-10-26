#include <entity.hpp>
#include <algorithm>
#include <physics/physics.hpp>

using namespace Engine;

std::shared_ptr<Entity> Entity::s_root = Entity::create("Root");

// creat the entity!
std::shared_ptr<Entity> Entity::create(std::string name) {
    // actually create it
    // i have been experimenting with std::move because i haven't really used it yet
    if (auto newEntity = std::shared_ptr<Entity>(new Entity(std::move(name)))) {
        // creation succeeded, initialise
        newEntity->_init();
        return newEntity;
    }
    return nullptr;
}

// was the transform dirty at any point this frame?
bool Entity::wasTransformDirty() {
    return m_transformDirty;
}

void Entity::_initQueuedComponents() {
    // while the queue has values in it
    while (m_componentInitQueue.size() != 0) {
        // get the front component
        auto component = m_componentInitQueue.front();
        // if the component pointer is invalid or init has run
        if (component.expired() || component.lock()->hasInitRun()) {
            // remove this element
            m_componentInitQueue.pop();
            continue;
        }
        component.lock()->initImpl();
    }
}

// basic initialisation
void Entity::_init() {
    // get a new entity ID from the pool
    m_id = s_entityPool.getNewID();
    log().logInfo("Creating new Entity with name {} and EntityID {}.", m_name, m_id);
    // add this entity to the entity list
    s_entityList.emplace(m_id, shared_from_this());
    // parent this to the root node.
    setParent(s_root.get());
    // assign the transform entity reference
    transform.m_entity = this;
    if (this != s_root.get()) m_layer = LayerRegistry::get().getLayerViaName(layer_defaultLayer)->m_layerID;
}

Entity::~Entity() {
    log().logInfo("Entity {} with EntityID {} was destroyed.", m_name, m_id);
    if (!m_released) {
        // if this entity hasn't been released yet, do that.
        release();
    }
}

// remove this instance from the parent's children vector
void Entity::_removeSelfFromParentChildren() {

    // get a shared ptr to this (easier. we don't really have to do this but it looks nicer)
    auto _this = shared_from_this();

    // get shared_ptr to parent (remember its a weak_ptr)
    if (auto parent = m_parent.lock()) {

        // get all children from parent 
        auto& pChildren = parent->m_children;

        // erase the child from the children vector if the child is this entity.
        std::erase_if(pChildren, [&_this](const std::weak_ptr<Entity>& weakChild) {

            // get a temporary shared_ptr from the weak_ptr
            auto shared = weakChild.lock();

            // contirm it is valid and that it is equal to this entity
            return shared && (shared == _this); // this is a boolean
        });

        // update the parent's other children
        parent->_updateChildrenIndices();
    }
}

EntityID Entity::getID() {
    return m_id;
}

// release this shared_ptr. (remove all references of it.)
void Entity::release() {
    
    if (this->weak_from_this().expired()) {
        // if shared_ptr is expired, shared_from_this will throw an error
        // and complain so we have to be safe.
        return; 
    }
    if (shared_from_this() == s_root) return; // don't delete the root entity 

    // release all children

    // loop through all children
    for (auto& child : m_children) {

        // if child is not null (also get the child)
        // (really just a "just in case" check)
        if (auto sharedChild = child.lock()) {

            // delete the child too.
            // we don't need to remove it 
            // from the child list,
            // it will do that itself in it's
            // release function.
            sharedChild->release();
        }
    }

    // remove this entity from the parent's child list (see)
    _removeSelfFromParentChildren();

    // remove this entity from the entity list
    s_entityList.erase(m_id);
    // release the entity id so it can be reused
    s_entityPool.releaseID(m_id);
    // released is now true. this is so the destructor doesn't call release again.
    m_released = true;
}

// update the entityIndexToParent of all children
void Entity::_updateChildrenIndices() {
    // loop through all children with a basic index loop
    for (size_t i = 0; i < m_children.size(); i++) {
        // if child is valid
        if (auto child = m_children[i].lock()) {
            // set the index to parent to the current loop index
            child->m_entityIndexToParent = i;
        }
    }
}

// get an entity via id.
Entity* Entity::getEntityViaID(EntityID id) {
    // get entity from map using id key
    auto iterator = s_entityList.find(id);
    if (iterator != s_entityList.end()) {
        return s_entityList[id].get(); // return raw ptr
    }
    // entity not found, return nullptr
    log().logError("Entity with ID {} doesn't exist!", id);
    return nullptr;
}

// get the parent
Entity* Entity::getParent() {
    // if parent is not null
    if (auto p = m_parent.lock()) {
        // return the parent raw pointer
        return p.get();
    }
    // parent not assigned, return nullptr
    return nullptr;
}

// get child by index
Entity* Entity::getChildAt(int index) {
    // if size is 1, 1st index is 0. 
    // to account, just add 1 to the index.
    if (m_children.size() > index + 1) {
        // vector is big enough for this potential
        // object, so return the object at index.
        return m_children[index].lock().get();
    }
    // index was bigger than the object.
    log().logError("On Entity {}, Index of {} was outside the bounds of the child vector, which has a size of {}.", m_id, index, m_children.size());
    return nullptr;
}

// get the number of children
size_t Entity::getChildCount() {
    return m_children.size();
}

// get child by first reference of name. Potentially
// inefficient if the entity has a lot of children.
Entity* Entity::getChildByName(std::string name) {
    // loop through all children with a basic index loop
    for (size_t i = 0; i < m_children.size(); i++) {
        // if child is valid
        if (auto child = m_children[i].lock()) {
            // if child name is correct
            if (child->m_name == name) {
                // return the child
                return child.get();
            }
        }
    }
    // no child was found.
    log().logError("On Entity {}, no child named {} was found.", m_id, name);
    return nullptr;
}

void Entity::setLayer(LayerID id,       bool cascade) {
    if (!LayerRegistry::get().getLayerViaID(id)) return; // return if the ID is not valid
    m_layer = id;

    // cascade down and register all children under this layer too.
    if (cascade) {
        for (const auto& e : m_children) {
            setLayer(id, true);
        }
    }
}
void Entity::setLayer(std::string name, bool cascade) {

    auto layer = LayerRegistry::get().getLayerViaName(name);
    if (!layer) return;

    m_layer = layer->m_layerID;

    // same thing, still setting these via id though.
    if (cascade) {
        for (const auto& e : m_children) {
            setLayer(m_layer, true);
        }
    }
}
void Entity::setLayer(Layer* layer,     bool cascade) {
    setLayer(layer->m_layerID, cascade);
}

// get the index of this entity in it's parent's child vector
size_t Entity::getIndex() {
    return m_entityIndexToParent;
}

// set this entity's parent
void Entity::setParent(Entity* e) {
    // if we are trying to set the parent to itself,
    // then uh dont do that
    if (e == this) return;

    // remove this entity from the parent's children vector 
    // (if this entity has a parent)
    _removeSelfFromParentChildren();

    // add this entity to the new parent's children vector
    e->m_children.push_back(shared_from_this());
    // set this entity's index to parent
    m_entityIndexToParent = e->getChildCount() - 1;

    // m_parent is a weak_ptr, e is a raw ptr.
    // get the shared_ptr from the entity list.
    // getting the shared_ptr this way is 
    // actually the cheapest way of doing this.
    // (O(logN))
    m_parent = s_entityList[e->m_id];
    _invalidateChildTransforms();
}

// call every update function on all components
void Entity::_updateComponents(double delta) {
    if (!m_enabled) return; // return early if entity is not enabled

    // loop through every component and call update.
    for (const auto& component : m_componentMap) {
        component.second->update(delta);
    }
    // update this entity's enabled state.
    // does nothing if the enabled state isnt queued to change.
    _updateEnabled();
}

// call every update function on all components
void Entity::_updatePhysicsComponents(double delta) {
    if (!m_enabled) return; // return early if entity is not enabled

    // loop through every component and call update.
    for (const auto& component : m_componentMap) {
        component.second->physicsUpdateImpl(delta);
    }
    // update this entity's enabled state.
    // does nothing if the enabled state isnt queued to change.
    _updateEnabled();
}

// call every lateUpdate function on all components
void Entity::_lateUpdateComponents(double delta) {
    if (!m_enabled) return;
    
    // loop through every component and call lateUpdate.
    for (const auto& component : m_componentMap) {
        component.second->lateUpdate(delta);
    }
    _updateEnabled();
}

void Entity::_pollComponentBehaviours(double delta) {
    // update all the components' update() functions. 
    for (const auto& e : Entity::s_entityList) {
        e.second->_updateComponents(delta);
    }
    // update all the lateupdate() functions.
    for (const auto& e : Entity::s_entityList) {
        e.second->_lateUpdateComponents(delta);
    }
    // update all the draw() functions.
    for (const auto& e : Entity::s_entityList) {
        e.second->_drawComponents();
    }
}

void Entity::_transformFlagReset() {
    // reset all the transformDirty flags.

    // stupidly i had this reset at the end of the above loop, 
    // but that could cause wasEntityTransformed()
    // to be false on every other frame in certain cases.

    // this drove me. up the fucking wall.
    // for FOUR hours.
    
    for (const auto& e : Entity::s_entityList) {
        if (e.second->m_queueTransformReset) {
            // set transformDirty to false, this is the end
            // of the frame as far as behaviour is aware.
            e.second->m_transformDirty = false; 
            e.second->m_queueTransformReset = false;
        }
    }
}

// get the real global matrix, taking physics into account
fm::mat4 Entity::getRealMatrix() {
    
    // the global matrix itself
    fm::mat4 global = transform.getGlobalMatrix();
    // if we didn't do physics this frame, return global as-is.
    if (!m_didPhysicsThisTick || !m_interpolatePhysics) return global;

    // we did physics this frame, so mix the matrices.
    // calculate the percentage of how long we've waited
    // for the next physics tick.
    float percent = (PhysicsImpl::physicsTick.count() / (1.0f / static_cast<float>(PhysicsImpl::tps)));
    // mix the pre physics update matrix with the current matrix.
    return m_physicsGlobalMatrixCache.mixed(global, percent);
};

void Entity::_pollPhysicsBehaviours(double delta) {
    // update all the components' physics() functions. 
    for (const auto& e : Entity::s_entityList) {
        // cache the matrix before we performed any physics calculations
        e.second->m_physicsGlobalMatrixCache = e.second->transform.getGlobalMatrix();
        // set this to false now, it only becomes true if we do anything.
        e.second->m_didPhysicsThisTick = false;
        e.second->_updatePhysicsComponents(delta);
    }
}

// call every draw function on all components
void Entity::_drawComponents() {
    if (!m_enabled) return;
    
    auto layer = LayerRegistry::get().getLayerViaID(m_layer);
    if (!layer || !layer->m_enabled) return; // if the layer is null for whatever reason

    // loop through every component and call drawImpl.
    // this sends the render commands to the renderer's
    // bucket array.
    for (const auto& component : m_componentMap) {
        component.second->drawImpl(layer);
    }
    _updateEnabled();
}

// return if this entity is enabled or not.
bool Entity::isEnabled() {
    return m_enabled;
}

// queue this entity to change it's enabled state at the end of the frame.
void Entity::setEnabled(bool enabled) {
    if (m_enabled != enabled) m_enabledDirty = true;
}

// force the entity to change it's enabled state immediately.
void Entity::forceSetEnabled(bool enabled) {
    m_enabled = enabled;
    if (m_enabled) _initQueuedComponents();
}

// update this entity's enabled state.
void Entity::_updateEnabled() {
    if (!m_enabledDirty) return; // no state change, return early.
    // state has been changed, apply the changes
    m_enabledDirty = false;
    m_enabled = !m_enabled;
    if (m_enabled) _initQueuedComponents();
}

// invalidate the transforms of every child
void Entity::_invalidateChildTransforms() {

    transform.m_globalDirty = true;
    // loop through every entity
    for (auto e : s_entityList) {
        e.second->transform.m_globalDirty = true;
    }
}