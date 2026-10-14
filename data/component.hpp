#pragma once
#include <unordered_map>
#include <iostream>
#include <idpool.hpp>
#include <memory>

namespace Engine {

    // forward declaration
    class Entity;

    using ComponentTypeID = uint32_t;

    class EntityComponent {
    private:

        ComponentTypeID m_typeID;
        
        Entity* m_entity;

        static inline IDPool<ComponentTypeID> s_componentTypePool;

    public:

        virtual ~EntityComponent() = default; 
    
        Entity& getEntity();
        const Entity& getEntity() const;
        #define entity this->getEntity()

        const ComponentTypeID getTypeID() const;
        template <typename T>
        static ComponentTypeID getComponentTypeID() {

            if (!std::is_base_of<EntityComponent, T>::value) {
                std::cerr << "Provided class is not a component!\n"; 
                return -1;
            }
            ComponentTypeID id = s_componentTypePool.getNewID();
            return id;

        }


        virtual void init() {};
        virtual void update(float delta) {};
        virtual void lateUpdate(float delta) {};
        virtual void onDestroy() {};
    

        friend class Entity;

    };
}