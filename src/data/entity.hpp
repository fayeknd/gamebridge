#pragma once
#include <unordered_map>
#include <iostream>
#include "idpool.hpp"
#include <memory>

namespace Engine {
 
    // an alias so we don't have to keep typing "uint32_t"
    // a uint is definitely large enough btw.
    using EntityID = uint32_t;

    constexpr const char* entityDefaultName = "Unnamed Entity";

    // Entities will just exist in memory in a tree of children and parents.
    // any components can be added and removed to any of them to do whatever 
    // we want.

    // weak_ptr gets used a lot here,
    // so just a note because you'll see it a lot:
    // someWeakPtr.lock() returns a shared_ptr 
    // to the object. it doesn't really make sense
    // why the function is actually called lock,
    // and imo it is a bit confusing. just know that
    // it is getting a valid pointer and not locking
    // anything. 
    
    class Entity : public std::enable_shared_from_this<Entity> {
    private:

        // this allocated entity ID.
        EntityID m_id;

        // these would have just been IDs, but accessing them is more efficient
        // this way. this does have the downside that a lot of entities' pointers
        // will be stored in 3 separate places, but pointers aren't very big anyway.
        
        // a weak_ptr is a reference to the object without incrementing.
        // a weak_ptr understands the object might not exist soon.
        
        // parent
        std::weak_ptr<Entity> m_parent;
        // list of children
        std::vector<std::weak_ptr<Entity>> m_children;

        // the index of this entity in the parent's child vector.
        size_t m_entityIndexToParent;

        // create the entity ID pool
        // responsible for allocating and deallocating IDs
        static inline IDPool<EntityID> s_entityPool; 
        // create the entity list (use the id as the key)
        // we DO want a shared_ptr here. if an entity somehow doesn't exist
        // in this list, it will automatically be destroyed. this ensures
        // if an entity exists, it is referenced in this unordered map.
        static inline std::unordered_map<EntityID, std::shared_ptr<Entity>> s_entityList;

        // determines if the entity has been released or not.
        bool m_released = false;

        // update the entityIndexToParent of all children
        void _updateChildrenIndices();
        // basic initialisation
        void _init();
        // remove this instance from the parent's children vector
        void _removeSelfFromParentChildren();

        // constructor w/ default name
        Entity() : m_name(entityDefaultName) {};
        // constructor with name, copied
        Entity(std::string name) : m_name(std::move(name)) {};

    public:

        
        ~Entity();

        // the root entity. this will be an object's parent
        // by default. keep in mind, while the entity system is designed
        // to ensure every entity has a parent (except the root), we 
        // should still write code that handles an instance where the
        // entity does not have a parent.
        static std::shared_ptr<Entity> s_root;

        // create and return a new entity
        static std::shared_ptr<Entity> create(std::string name = entityDefaultName);

        // release this shared_ptr. (remove all references of it.)
        void release();
        
        // a basic entity name.
        std::string m_name;

        // get an entity via id.
        static Entity* getEntityViaID(EntityID id);

        // get entity id
        EntityID getID();

        // get the parent
        Entity* getParent();

        // get child by index
        Entity* getChildAt(int index);

        // get the number of children
        size_t getChildCount(); 

        // get child by first reference of name. Potentially
        // inefficient if the entity has a lot of children.
        Entity* getChildByName(std::string name);

        // get the index of this entity in it's parent's child vector
        size_t getIndex();

        // set this entity's parent
        void setParent(Entity* e);

    };

}

