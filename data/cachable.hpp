#pragma once
#include <memory>
#include <idpool.hpp>
#include <engine.h>
#include <unordered_map>

namespace Engine {

    // the cache id.
    // supports like 4 billion unique ids or something
    using cID = uint16_t;

    // the Cachable interface is responsible for storing 
    // all instances that derive from it in a map of weak_ptrs.
    // the reason we use a map is - while it is  slower than
    // a regular vector at getting an item at an index - it
    // does not have to re-set any "thisindex" variables
    // on all classes. (with this, we would have to iterate
    // through every instance if we wanted to or not.)
    class ICachable : public std::enable_shared_from_this<ICachable> {
    protected:

        // the instance ID pool.
        static inline IDPool<cID> s_instanceIDPool;
        
        // the map of cached instances
        static inline std::unordered_map<cID, std::weak_ptr<ICachable>> s_instanceCacheMap;
        
        // the internal cache id, per-instance.
        cID m_cacheID; 

        // register this instance to the cache
        void _registerPersistent();
        // opposite
        void _unregisterPersistent();


    public:

        virtual ~ICachable() = default;

    };

}