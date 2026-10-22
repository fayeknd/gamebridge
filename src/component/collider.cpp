#include <component/collider.hpp>

using namespace Engine;

void BoundingBox::init()
{
    s_boundingBoxList.push_back(weak_from_this());
}

bool BoundingBox::CheckBoxIntersection(const BoundingBox& a, const BoundingBox& b)
{
    return (a.m_min.x <= b.m_max.x && a.m_max.x >= b.m_min.x) &&
        (a.m_min.y <= b.m_max.y && a.m_max.y >= b.m_min.y) &&
        (a.m_min.z <= b.m_max.z && a.m_max.z >= b.m_min.z);
}

bool BoundingBox::CheckRayIntersection(const Ray& ray, RaycastHit& outRaycastHit)
{
    fm::vec3 origin = ray.m_origin.transformed(outRaycastHit.m_hitEntity->transform.getGlobalMatrix());
    fm::vec3 direction = ray.m_direction.transformed(outRaycastHit.m_hitEntity->transform.getGlobalMatrix());

    Ray localRay(origin, direction); 
    BoundingBox& boundingBox = *outRaycastHit.m_hitEntity->getComponent<BoundingBox>();

    if (RayAABoxIntersection(localRay, boundingBox, outRaycastHit.m_hitInfo))
    {
        Logging::Logger::get().logInfo("Raycast Hit At {}", boundingBox.getEntity().m_name);
        outRaycastHit.m_hitInfo.m_hasHit = true;
    }

    return outRaycastHit.m_hitInfo.m_hasHit;
}

bool BoundingBox::RayAABoxIntersection(const Ray& ray, const BoundingBox& box, HitInfo& outHitInfo)
{
    fm::vec3 tMin = (box.m_min - ray.m_origin) / ray.m_direction;
    fm::vec3 tMax = (box.m_max - ray.m_origin) / ray.m_direction;

    // Find the nearest and farthest intersections
    fm::vec3 tNear = fm::vec3::min(tMin, tMax);
    fm::vec3 tFar = fm::vec3::max(tMin, tMax);

    // Find the maximum of the nearest intersections
    float tNearMax = std::max(std::max(tNear.x, tNear.y), tNear.z);
    // Find the minimum of the farthest intersections
    float tFarMin = std::min(std::min(tFar.x, tFar.y), tFar.z);

    // Check if the ray intersects the bounding box
    if (tNearMax <= tFarMin && outHitInfo.m_distance > tNearMax)
    {
        // Ray intersects the bounding box
        // Calculate the intersection point
        fm::vec3 intersectionPoint = ray.m_origin + ray.m_direction * tNearMax;
        outHitInfo.m_hitPoint = intersectionPoint;
        outHitInfo.m_distance = tNearMax;
        outHitInfo.m_hasHit = true;

        return true;
    }

    return false;
}

void BoundingBox::SetBounds(fm::vec3 min, fm::vec3 max)
{
    m_min = min;
    m_max = max;
}


