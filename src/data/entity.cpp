#include "entity.hpp"
#include <algorithm>

using namespace Engine;

// assign the root entity
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

// basic initialisation
void Entity::_init() {
    // get a new entity ID from the pool
    m_id = s_entityPool.getNewID();
    // add this entity to the entity list
    s_entityList.emplace(m_id, shared_from_this());
    // parent this to the root node.
    setParent(s_root.get());
}

Entity::~Entity() {
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
    std::cerr << "Entity with ID " << id << " doesn't exist!\n";
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
    std::cerr << "On Entity " << m_id << ", getParent was called but no parent exists.\n";
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
    std::cerr << "On Entity " << m_id << ", Index of " << index << " was outside the bounds of the child vector, which has a size of " << m_children.size() << ".\n";
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
    std::cerr << "On Entity " << m_id << ", no child named " << name << " was found.\n";
    return nullptr;
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
}