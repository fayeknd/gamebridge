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

    // a vector to hold the IDs of objects we need to delete
    std::vector<CacheID> objectsToDelete;
    
    // loop through 
    for (const auto& object : registry()) {
        // if the instance total references is less than or equal to 1,
        // (that 1 being the cache itself) and the object is not marked
        // as don't delete, then is viable for cleanup.
        if (object.second.use_count() <= 1 && !object.second->m_dontDelete) {
            // add this object to the objectsToDelete vector.
            objectsToDelete.push_back(object.first);
        }
    }
    // no objects to delete, return early.
    if (objectsToDelete.size() == 0) return;

    // NOW loop through all the objects marked for deletion.
    for (unsigned int id : objectsToDelete) {
        auto it = registry().find(id);
        if (it != registry().end()) {
            // found an object, unregister it.
            it->second->_unregister();
            erasedObjs++;
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

    ObjectRegistry::cleanup();
    return;


    // begin the timer if it hasn't started
    if (!s_gcTimer.isCounting()) s_gcTimer.start();
    
    // is the timer past the cleanup interval?
    if (s_gcTimer.count() >= s_cleanupInterval) {
        // yes, cleanup and reset the timer
        ObjectRegistry::cleanup();
        s_gcTimer.reset(); 
    }
}