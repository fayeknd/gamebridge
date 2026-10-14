#pragma once
#include <unordered_map>
#include <iostream>
#include "idpool.hpp"
#include <memory>

namespace Engine {

    // forward declaration
    class Entity;

    using ComponentTypeID = uint32_t;

    class EntityComponent {
    private:

        const char* m_typename; 
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
        static ComponentTypeID getComponentTypeID();

        virtual void init();
        virtual void update(float delta);
        virtual void lateUpdate(float delta);
        virtual void onDestroy(float delta);
    

        friend class Entity;

    };
}