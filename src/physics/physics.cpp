#include <physics/physics.hpp>
#include <logger.hpp>
#include <component/boundingbox.hpp>
#include <vector>
#include <entity.hpp>

using namespace Engine;
using namespace Physics;


RaycastHit Physics::Raycast(Ray ray, LayerMask mask, float distance) {

    RaycastHit returnHit;
    returnHit.m_hitInfo.m_mask = mask;
    // iterate through every bounding box (expensive!!)
    for (auto it : BoundingBox::getBoundingBoxList()) {

        // get the bounding box pointer
        BoundingBox* bb = it.second;

        // check this entity is not blacklisted from the layer mask
        if ((bb->getEntity().getLayerID() & mask) == 0 || bb->getEntity().getLayer()->m_ignoreRaycast) {
            continue;
        }

        // set some hit values
        RaycastHit hit;
        hit.m_hitEntity = &bb->getEntity();
        hit.m_hitBox    = bb;

        // flood raycasthit with hit info
        bb->checkRayIntersection(ray, hit);

        // did we hit?
        // we hit bitches - hunter
        if(hit.m_hitInfo.m_distance < returnHit.m_hitInfo.m_distance)
            returnHit = hit;
    }

    return returnHit;
}

void PhysicsImpl::pollPhysics()
{
    // begin the timer if it hasn't started
    if (!physicsTick.isCounting()) physicsTick.start();
    double d = 1.0 / static_cast<double>(tps);

    // is the timer past the tick interval?
    if (physicsTick.count() >= d) {
        // yes, push through physics poll and reset the timer
        Entity::_pollPhysicsBehaviours(d);
        physicsTick.reset(); 
    }
}