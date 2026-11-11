#pragma once
#include <unordered_map>
#include <iostream>
#include <memory>
#include <cstdint>
#include <transform.hpp>
#include <logger.hpp>
#include <layer.hpp>
#include <material.hpp>
#include <field_reflect.hpp>

namespace Engine {

    // forward declaration
    static inline void initializeReflectionData();

    // forward declaration to prevent
    // circular dependency issues.
    // in any component cpp file,
    // remember to include <entity.hpp>
    class Entity;

    // just to make it clearer what a function is working with
    using ComponentTypeID = uint16_t;

    // Component interface / abstract class.
    // Components are assigned to entities
    // and stored in a component map.
    // The components are kept track of via the
    // ComponentTypeID value. This value is 
    // assigned via a static template function.
    // More on that below

    class IComponent {
    public:
        friend void Engine::initializeReflectionData();
    private:

        static Logging::Logger& log() {
            static Logging::Logger instance("COMPONENT");
            return instance;
        }

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

        // has this component's init() been run?
        bool m_hasInit;

        // tell the instance to run draw(). 
        static inline void _distributeDraw(IComponent* instance) { instance->draw(); }

    protected:

        // ignore this
        static inline std::string_view _reflectionName = "IComponent";
        static inline const std::vector<ReflectableField>& (*_reflectionFieldQuery)() = nullptr;


        // init() is called when the component is added to an entity, once.
        virtual void init() {};
        // final stage in the game loop. used for anything to do with drawing.
        virtual void draw() {};

        // actual physics update function
        virtual void physicsUpdate(double delta) {};

    public:

        virtual std::vector<std::pair<ReflectableField, void*>> getFields() = 0;
        virtual std::string_view getTypename() const = 0;

        using ReflectionQueryFunc = const std::vector<ReflectableField>& (*)();
        
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
        static ComponentTypeID getComponentID() {
            // create the static typeid. never gets created again.
            static const ComponentTypeID id = _nextID();
            return id;
        }

        // material(s) to use
        // this was moved here so we don't have to dyanmic
        // cast to an IRenderable when sending render data.
        // if the material is null, no render data.
        // else, we can determine if the material is transparent,
        // and use the appropriate depth function.
        std::vector<std::shared_ptr<Material>> m_materials;
        #define m_material m_materials[0]


        // marks if this instance should render or not 
        // (given it can even be rendered)
        PUBLIC bool m_render = true;

        // returns if the init function has been run yet.
        bool hasInitRun(); 

        virtual ~IComponent() = default; 

        // we don't want the entity to be written to
        Entity& getEntity();
        // const getter instead
        const Entity& getEntity() const;

        // we use this to make it look nicer syntactically.
        // this enables us to use entity.doSomething()
        // without entity being writable. just feels nicer to use.
        #define entity this->getEntity()

        // makes it a bit easier to run the base initialiser
        #define baseinit IComponent::init()
        
        // get this instance's typeid.
        const ComponentTypeID getTypeID() const;

        // public method of getting a component type id.
        // aquired using IComponent::getComponentTypeID<MyAwesomeComponent>();
        template <typename T>
        static ComponentTypeID getComponentTypeID() {

            // check if type is actually a derivative of IComponent
            if (!std::is_base_of<IComponent, T>::value) {
                // type is not a component, return.
                log().logError("(getComponentTypeID<T>) Provided class is not a component!");
                return -1;
            }
            // return the unique ID for this class
            return getComponentID<T>();

        }

        // external init function. this is the externally visible
        // init function. used so each component does not have to call
        // the base init to have m_hasInit set to true, while calling
        // init will still set it to true. useful!
        void initImpl() {
            // set hasInit to true
            m_hasInit = true;
            // run the init function
            init();
        }


        // virtual basic behaviour functions.

        // update is obvious, called once per frame. delta time is passed through.
        // i know unity does not pass through delta time, but engines like 
        // cocos2d-x and godot do this, and it's helpful in those. 
        virtual void update(double delta) {};

        // lateUpdate is the second stage of update and all lateUpdate functions
        // are called after all update functions are finished calling.
        virtual void lateUpdate(double delta) {};

        // physics update calls an update every 50ms (same as minecraft yippee)
        void physicsUpdateImpl(double delta);

        // adds draw() to the render bucket callback in renderer.hpp
        void drawImpl(const Layer* layer);

        // called right before the object gets destroyed. (or dereferenced, 
        // the object won't be destroyed if there is a shared_ptr to it.
        // this function will still run in that scenario though. don't store
        // a shared_ptr to any component.)
        virtual void onDestroy() {};

    

        // allow the Entity class full access to a component's functions.
        friend class Entity;

    };
}

// use this instead of "public:" in components
#define component_public \
    public: \
        friend void Engine::initializeReflectionData(); \
\
    protected: \
        static inline std::string_view _reflectionName = ""; \
        static inline const std::vector<Engine::ReflectableField>& (*_reflectionFieldQuery)() = nullptr; \
\
    public: \
        std::string_view getTypename() const override { \
            if (!_reflectionName.empty()) return _reflectionName; \
\
            return "UninitialisedComponent"; \
        } \
        std::vector<std::pair<ReflectableField, void*>> getFields() override { \
            using ComponentType = std::decay_t<std::remove_pointer_t<decltype(this)>>; \
            std::vector<std::pair<ReflectableField, void*>> _; \
            if (!_reflectionFieldQuery) return _; \
            auto& fields = _reflectionFieldQuery(); \
            for (Engine::ReflectableField field : fields) { \
                auto typedPtr = Reflection::getFieldPtr<ComponentType>(this, field); \
                void* genericVoidPtr = const_cast<void*>(static_cast<const void*>(typedPtr)); \
                _.push_back(std::make_pair(field, genericVoidPtr)); \
            } \
            return _; \
        } public
        