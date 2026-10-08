#pragma once 

#include <vector>
#include <stdexcept>
#include <iostream>
#include <stdint.h>

namespace Engine {

    // ok first class im documenting lol

    // so an ID pool is an object that can assign an id of any type. (T.)
    // The purpose of this is that an object can be assigned an ID with aquireUniqueID()
    // and when that object is destroyed, the ID it was using is freed.
    // For instance, if an entity is created, it gets a new unique ID. 
    // when that entity is destroyed, that ID is released into the pool for use again.
    // the m_freeIDs vector is prioritised when assigining a new id, and is only
    // added to when an ID is released.

    // just let this be a uint32_t by default.
    template <typename T = uint32_t>
    class IDPool {
    private:

        // next available ID
        T m_nextID;
        // a list of the available IDs for use.
        std::vector<T> m_freeIDs;

    public:

        // default constructor sets the nextID to 0.
        IDPool() : m_nextID(0) {};

        // self-explanitory. just clear the whole pool. 
        void clear(); //(idpool.cpp:8)

        // return the value of currently assigned IDs. 
        size_t count() const; //(idpool.cpp:17)

        // get a unique ID from the pool. 
        T getID(); //(idpool.cpp:25)

        // free an ID. this should be called in the destructors of anything that uses an IDPool.
        void releaseID(T uid); //(idpool.cpp:42)
    };
}