#include "idpool.hpp"

using namespace Engine;

// self-explanitory. just clear the whole pool.

template <typename T>
void IDPool<T>::clear() {
    
    m_nextID = 0;
    m_freeIDs.clear();

}

// return the value of currently assigned IDs. 
template <typename T>
size_t IDPool<T>::count() const {
    
    return m_nextID - m_freeIDs.size();

}

// get a unique ID from the pool.
template <typename T>
T IDPool<T>::getID() {
    // the freeIDs list is not empty, prioritise picking an ID from here.
    if (m_freeIDs.size() != 0) {

        // get the oldest freed ID.
        T uid = m_freeIDs.back();
        // then remove it.
        m_freeIDs.pop_back();
        return uid;
        
    }
    // list was empty, create a new ID.
    return m_nextID++;
}

// free an ID. this should be called in the destructors of anything that uses an IDPool. 
template <typename T>
void IDPool<T>::releaseID(T uid) {

    // if uid is less than or equal to nextID, then the uid cannot have possibly been assigned to anything.
    if (uid >= m_nextID) {
        throw std::invalid_argument("Attempted to release an ID that has not been allocated yet!");
    }

    // add this id to the list of free ones. 
    m_freeIDs.push_back(uid);
}