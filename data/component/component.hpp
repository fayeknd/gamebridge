#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <transform.hpp>

namespace Engine {

    // forward declaration to prevent
    // circular dependency issues.
    // in any component cpp file,
    // remember to include <entity.hpp>
    class Entity;

    // just to make it clearer what a function is working with
    using ComponentTypeID = uint32_t;

    // Component interface / abstract class.
    // Components are assigned to entities
    // and stored in a component map.
    // The components are kept track of via the
    // ComponentTypeID value. This value is 
    // assigned via a static template function.
    // More on that below

    class IComponent {
    private:

        // makes it easier to aquire the typeid for each
        // component instance. (this is assigned when adding
        // a component to an entity, and not handled by the
        // component itself.)
        ComponentTypeID m_typeID;
        
        // a pointer to the entity this component is a part of.
        Entity* m_entity;

        // get the next available ID for a component type.
        // this is not an IDPool because component types do
        // not get discarded at runtime.
        static ComponentTypeID _nextID();

        // get the typeid. this is unique for each component type.
        // the way this works: if this is called on a component
        // with the type of "PlayerComponent", it will create and
        // return a brand new ID on the first call, and return the same
        // on following calls. this means every type of component
        // gets a unique but consistent ID.

        // The reason this works is because the function is a template
        // which is like a blueprint class and is not actually static
        // for the whole "IComponent" class and each derivative of
        // IComponent  will have it's own version of this template.
        // this is kind of confusing i understand, i just about 
        // understand it enough to take advantage of it but explaining it
        // is kinda hard.

        // make sure this is a template 
        template <typename T>
        static ComponentTypeID _getID() {
            // create the static typeid. never gets created again.
            static const ComponentTypeID id = _nextID();
            return id;
        }

    public:

        virtual ~IComponent() = default; 

        // we don't want the entity to be written to
        Entity& getEntity();
        // const getter instead
        const Entity& getEntity() const;

        // we use this to make it look nicer syntactically.
        // this enables us to use entity.doSomething()
        // without entity being writable. just feels nicer to use.
        #define entity this->getEntity()

        // get this instance's typeid.
        const ComponentTypeID getTypeID() const;

        // public method of getting a component type id.
        // aquired using IComponent::getComponentTypeID<MyAwesomeComponent>();
        template <typename T>
        static ComponentTypeID getComponentTypeID() {

            // check if type is actually a derivative of IComponent
            if (!std::is_base_of<IComponent, T>::value) {
                // type is not a component, return.
                std::cerr << "Provided class is not a component!\n"; 
                return -1;
            }
            // return the unique ID for this class
            return _getID<T>();

        }

        // virtual basic behaviour functions.

        // init() is called when the component is added to an entity, once.
        virtual void init() {};

        // update is obvious, called once per frame. delta time is passed through.
        // i know unity does not pass through delta time, but engines like 
        // cocos2d-x and godot do this, and it's helpful in those. 
        virtual void update(double delta) {};

        // lateUpdate is the second stage of update and all lateUpdate functions
        // are called after all update functions are finished calling.
        // used for things like rendering. "late-stage" game loop stuff.
        virtual void lateUpdate(double delta) {};

        // called right before the object gets destroyed. (or dereferenced, 
        // the object won't be destroyed if there is a shared_ptr to it.
        // this function will still run in that scenario though. don't store
        // a shared_ptr to any component.)
        virtual void onDestroy() {};
    

        // allow the Entity class full access to a component's functions.
        friend class Entity;

    };
}