#pragma once
#include <component/component.hpp>
#include <idpool.hpp>
#include <queue>

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

        // if disabling this entity, we should still run all the 
        // update functions for that frame and then let it be disabled on the next.
        // this might be kind of stupid. if this needs to change lmk
        bool m_enabled = true;
        // mark if the "enabled" state has changed
        bool m_enabledDirty = false;

        // specifically for any cameras. effectively m_dirty
        // but doesn't reset when the transform matrix gets 
        // computed. just a check to see if the transform
        // was dirty at any point this frame.
        bool m_transformDirty = true;

        // update the enabled state.
        void _updateEnabled();

        // this entity's component map. access a component via a componentTypeID.
        std::unordered_map<ComponentTypeID, std::shared_ptr<IComponent>> m_componentMap;

        // this entity's component queue, for any components that have not been initialised yet.
        // this is required because if an entity is disabled, components will not run init().
        // eventually they need to run init, so put them in a queue. when this entity is enabled
        // again, iterate through this queue and run init().
        std::queue<std::weak_ptr<IComponent>> m_componentInitQueue;

        // run the init function on the queued components
        void _initQueuedComponents();

        // call every update function on all components
        void _updateComponents(double delta);

        // call every lateUpdate function on all components
        void _lateUpdateComponents(double delta);

        // call every draw function on all components
        void _drawComponents();

        // poll the behaviour of all components
        static void _pollComponentBehaviours(double delta);

        // invalidate all the child transforms
        void _invalidateChildTransforms(); 
    public:
    
        // queue the m_transformDirty var to get reset
        bool m_queueTransformReset = false;

        // was the transform dirty at any point this frame?
        bool wasTransformDirty();

        // this entity's transform. by itself, doesn't actually do anything.
        // only useful when utilised by things such as a rendering component.
        Transform transform;

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

        // add a component to this entity.
        // if that entity has a constructor, pass constructor
        // args through as parameters. for example: 
        // auto player = entity->addComponent<Player>("PlayerName", playerHealth, canDoubleJump);
        template <typename T, typename ... Args>
        T* addComponent(Args&& ... targs) {
            // create the component instance itself.
            // std::forward will just forward the arguments to any
            // constructor present in the given component class.
            auto component = std::make_unique<T>(std::forward<Args>(targs)...);
            // assign the component's entity pointer to this.
            component->m_entity = this;

            // get the component type id
            ComponentTypeID id = IComponent::getComponentTypeID<T>();
            // assign the component it's type id for easy internal lookup
            component->m_typeID = id;

            // move the component to the component map
            m_componentMap[id] = std::move(component);
            // if this entity is enabled, run the init function.
            if (m_enabled) {
                m_componentMap[id]->initImpl();
            }
            else {
                // not enabled, queue the init function for later.
                m_componentInitQueue.push(m_componentMap[id]);
            }
            // return a pointer to the component we just made.
            // allows for instant assignment.
            // (auto c = e->addComponent<Component>(), c is the component) 
            return static_cast<T*>(m_componentMap[id].get());
        }

        // get a component from the entity.
        template <typename T>
        T* getComponent() {

            // try find a component with the type given in the component man
            auto it = m_componentMap.find(IComponent::getComponentTypeID<T>());
            if (it != m_componentMap.end()) {
                // component found
                
                // we know what type of component this is
                // (static cast is much cheaper than dynamic_cast)
                return static_cast<T*>(it->second.get());
            }

            // no component was found.
            return nullptr;
        }

        // remove a component from 
        template <typename T> 
        void removeComponent() {

            // try find the component
            auto it = m_componentMap.find(IComponent::getComponentTypeID<T>());
            if (it != m_componentMap.end()) {

                // component found. get a pointer to it
                auto component = m_componentMap[it];

                // call the destroy function
                component->onDestroy();
                // just in case the entity isn't destroyed, unhook it
                // from the entity.
                component->m_entity = nullptr;

                // erase the component from the map.
                // if there are no shared_ptrs to this component,
                // it will just automatically deallocate.
                m_componentMap.erase(it);
                return; 
            }
            // no component was found.
            std::cerr << "No component of that type was found on entity " << m_name << ".\n";
        }

        // return if this entity is enabled or not
        bool isEnabled();

        // queue this entity to change it's enabled state at the end of the frame.
        void setEnabled(bool enabled);

        // force the entity to change it's enabled state immediately.
        void forceSetEnabled(bool enabled);

        // update loop happens in window, allow private function calling from
        // the window class.
        friend class Window; 
        // let the transform call _invalidateChildTransforms();
        friend class Transform;

    };

}

