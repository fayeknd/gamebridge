#include <physics.hpp>
#include <component/collider.hpp>
#include <logger.hpp>
#include <vector>

using namespace Engine;


RaycastHit Physics::Raycast(Ray ray, RaycastHit& rayCastHit)
{
    Logging::Logger::get().logInfo("Raycast casted");

    for (size_t i = 0; i < BoundingBox::s_boundingBoxList.size(); i++)
    {
        auto bb = BoundingBox::s_boundingBoxList[i];
        if (bb.expired()) 
        {
            BoundingBox::s_boundingBoxList.erase(BoundingBox::s_boundingBoxList.begin() + i);
            continue;
        }
 

        RaycastHit hit;
        hit.m_hitEntity = &bb.lock().get()->getEntity();

        bb.lock()->CheckRayIntersection(ray, hit);

        if(hit.m_hitInfo.m_distance < rayCastHit.m_hitInfo.m_distance)
            rayCastHit = hit;
    }

    return rayCastHit;
}