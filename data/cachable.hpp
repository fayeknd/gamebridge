#pragma once
#include <memory>
#include <idpool.hpp>
#include <engine.h>
#include <unordered_map>
#include <logger.hpp>

namespace Engine { 

    // the cache id.
    // supports like 4 billion unique ids or something
    using CacheID = uint16_t;

    // the Cachable base interface is responsible for storing 
    // all instances that derive from it in a map of weak_ptrs.
    // the reason we use a map is - while it is  slower than
    // a regular vector at getting an item at an index - it
    // does not have to re-set any "thisindex" variables
    // on all classes. (with this, we would have to iterate
    // through every instance if we wanted to or not.)
    class ObjectRegistry : public std::enable_shared_from_this<ObjectRegistry> {
    private:
        // the internal cache id, per-instance.
        CacheID m_cacheID; 
 
        // has this instance been registere?
        bool m_registered = false;
    protected:

        // the instance ID pool.
        static IDPool<CacheID>& instanceIDPool();
        
        // the map of cached instances
        static std::unordered_map<CacheID, std::shared_ptr<ObjectRegistry>>& registry();

        // destructor
        virtual ~ObjectRegistry() = default;
        ObjectRegistry() : m_cacheID(instanceIDPool().getNewID()) {};

        // register this instance to the registry
        void _register();

        // unregister this instance from the registry
        void _unregister();
 
        // garbage collection timer
        static inline Timer s_gcTimer;

    public:
        // marks if the registry has changed.
        static inline bool s_logGC = true;

        // how often should we collect garbage data?
        // (300s is 5 minutes)
        static inline double s_cleanupInterval = 300;
        static void pollGC();

        static Logging::Logger& log() {
            static Logging::Logger instance("REGISTRY");
            return instance;
        }  
        static void cleanup(); 
        CacheID getCacheID();
        bool getRegistered();

        // if this is true, this object won't get cleared.
        bool m_dontDelete = false;

    };

    // the actual ICachable class is unique for each TYPE of object. 
    // this means it can cast it the derivative type to the base class map.
    template <typename T>
    class ICachable : public ObjectRegistry {
    public:

        ICachable() : ObjectRegistry() {};
        // get the cached instance of an object.
        static std::shared_ptr<T> getCachedInstance(CacheID cid) {

            // try find an item in the registry with the cacheid
            auto it = registry().find(cid);
            
            // was the item found?
            if (it != registry().end()) {

                // yes! try cast it to our type.
                // dynamic_cast isn't very efficient, but 
                // if we provided the cacheid to a different
                // object type, it would crash. and we 
                // shouldn't really be calling this function
                // often anyway.
                // (static_cast just crashes if it fails, 
                // and doesn't throw an error.)
                auto inst = dynamic_pointer_cast<T>(it->second); 
                
                // is instance null?
                if (!inst) {
                    // instance is null, cast was incorrect.
                    ObjectRegistry::log().logError("CacheID {} does not point to a value with the specified type!", cid);
                    return nullptr;
                }
                // instance was not null, we have an instance to return

                return inst;
            }

            // no object was found. the cacheid was invalid.
            ObjectRegistry::log().logError("CacheID {} does not point to a valid cached instance!", cid);
            return nullptr;
        }

    };

}