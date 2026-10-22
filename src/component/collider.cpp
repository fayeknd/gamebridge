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
    float tNearMax = -std::numeric_limits<float>::max();
    float tFarMin = std::numeric_limits<float>::max();

    // Test intersection for each axis (X, Y, Z) safely handling negative directions
    for (int i = 0; i < 3; ++i)
    {
        float origin = ray.m_origin[i];
        float dir = ray.m_direction[i];
        float minVal = box.m_min[i];
        float maxVal = box.m_max[i];

        // Avoid division by zero if ray is parallel to the slab
        if (std::abs(dir) < 1e-8f)
        {
            // If origin is outside the slab along this parallel axis, no intersection
            if (origin < minVal || origin > maxVal)
                return false;
        }
        else
        {
            float t1 = (minVal - origin) / dir;
            float t2 = (maxVal - origin) / dir;

            if (t1 > t2) std::swap(t1, t2);

            tNearMax = std::max(tNearMax, t1);
            tFarMin = std::min(tFarMin, t2);

            // Box is missed if near intersection is greater than far intersection
            if (tNearMax > tFarMin)
                return false;
        }
    }

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


