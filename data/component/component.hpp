#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <transform.hpp>

namespace Engine {

    // forward declaration
    class Entity;

    using ComponentTypeID = uint32_t;

    class IComponent {
    private:

        ComponentTypeID m_typeID;
        
        Entity* m_entity;

        static ComponentTypeID _nextID();

        template <typename T>
        static ComponentTypeID _getID() {
            static const ComponentTypeID id = _nextID();
            return id;
        }

    public:

        virtual ~IComponent() = default; 
    
        Entity& getEntity();
        const Entity& getEntity() const;
        #define entity this->getEntity()

        const ComponentTypeID getTypeID() const;

        template <typename T>
        static ComponentTypeID getComponentTypeID() {

            if (!std::is_base_of<IComponent, T>::value) {
                std::cerr << "Provided class is not a component!\n"; 
                return -1;
            }
            return _getID<T>();

        }


        virtual void init() {};
        virtual void update(float delta) {};
        virtual void lateUpdate(float delta) {};
        virtual void onDestroy() {};
    

        friend class Entity;

    };
}