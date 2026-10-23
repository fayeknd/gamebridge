#include <physics.hpp>
#include <logger.hpp>
#include <component/boundingbox.hpp>
#include <vector>

using namespace Engine;
using namespace Physics;


RaycastHit Physics::Raycast(Ray ray, RaycastHit& rayCastHit, float distance) {

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
        // we hit bitches - hunter
        if(hit.m_hitInfo.m_distance < rayCastHit.m_hitInfo.m_distance)
            rayCastHit = hit;
    }

    return rayCastHit;
}