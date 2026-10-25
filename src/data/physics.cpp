#include <physics.hpp>
#include <component/collider.hpp>
#include <logger.hpp>
#include <vector>

using namespace Engine;


RaycastHit Physics::Raycast(Ray ray, float distance) {

    RaycastHit returnHit;
    // iterate through every bounding box (expensive!!)
    for (auto it : BoundingBox::getBoundingBoxList()) {

        // get the bounding box pointer
        auto bb = it.second;

        // set some hit values
        RaycastHit hit;
        hit.m_hitEntity = &bb->getEntity();
        hit.m_hitBox    = bb;

        // flood raycasthit with hit info
        bb->checkRayIntersection(ray, hit);

        // did we hit?
        // (this is weird, im so sure that raycast article was ai)
        if(hit.m_hitInfo.m_distance < returnHit.m_hitInfo.m_distance)
            returnHit = hit;
    }

    return returnHit;
}