#include <cachable.hpp>

using namespace Engine;

// same as below.
IDPool<CacheID>& ObjectRegistry::instanceIDPool() {
    static auto* pool = new IDPool<CacheID>();
    return *pool;
}

// the program cache stores a shared_ptr to the shader, and a string as an identifier. (m_key)
std::unordered_map<CacheID, std::shared_ptr<ObjectRegistry>>& ObjectRegistry::registry() {
    // create regsitry
    // this is actually purposefully leaking memory
    // but it's okay because it will be deallocated
    // when the program terminates.
    // it leaks to MAKE SURE it is deleted after
    // other objects.
    // this is kind of evil though and i hate it and im sorry
    static auto* cache = new std::unordered_map<CacheID, std::shared_ptr<ObjectRegistry>>();
    return *cache;
}

void ObjectRegistry::cleanup() {
    // track the erased object count
    int erasedObjs = 0;

    // loop through every item in the cache
    for (auto it = registry().begin(); it != registry().end();) {
        // if the instance total references is less than or equal to 1,
        // (that 1 being the cache itself) and the object is not marked
        // as don't delete, then is viable for cleanup.
        if (it->second.use_count() <= 1 && !it->second->m_dontDelete) {
            // delete the instance.
            // this forces the count to drop to 0
            // and the smart pointer automatically
            // deletes the program. 
            it->second->_unregister();
            erasedObjs++; // we erased an object, increment.
        }
        else {
            // this instance is referenced, 
            // continue to the next instance.
            it++;
        }
    }
    // if erased objects is at least 1 and logging is enabled for GC
    if (erasedObjs > 0 && s_logGC) {
        log().logInfo("Cleaned up {} unused ref(s).", erasedObjs);
    } 
}

// register this instance to the registry
void ObjectRegistry::_register() {
    log().logInfo("Registering new cached instance with CacheID {}.", m_cacheID);
    // the constructor generates a new id, we don't have to worry about that.
    registry().emplace(m_cacheID, shared_from_this());
    m_registered = true; 
}

// unregister this instance from the registry
void ObjectRegistry::_unregister() {
    log().logInfo("Unregistering cached instance with CacheID {}.", m_cacheID);
    // remove this from the registry
    registry().erase(m_cacheID);
    // release the cache ID for another instance to use
    instanceIDPool().releaseID(m_cacheID);
}

// run the garbage collector every x minutes
void ObjectRegistry::pollGC() {
    // begin the timer if it hasn't started
    if (!s_gcTimer.isCounting()) s_gcTimer.start();
    
    // is the timer past the cleanup interval?
    if (s_gcTimer.count() >= s_cleanupInterval) {
        // yes, cleanup and reset the timer
        ObjectRegistry::cleanup();
        s_gcTimer.reset(); 
    }
}