#pragma once
#include <fm/fmath.hpp>
#include <entity.hpp>

namespace Engine
{
    // Data for what the raycast hit, whether the value was a hit, the distance of the hit, where the hit was on the collider
    // and the normal of the hit
    struct HitInfo
    {
        bool m_hasHit = false;
        float m_distance = std::numeric_limits<float>::max();
        fm::vec3 m_hitPoint;
        fm::vec3 m_normal;
    };

    struct RaycastHit
    {
        HitInfo m_hitInfo;
        Entity* m_hitEntity;
        RaycastHit() = default;

        explicit operator bool() const noexcept
        {
            return (m_hitEntity != nullptr);
        }
    };

    struct Ray
    {
        fm::vec3 m_origin;
        fm::vec3 m_direction;
    };

    
    // Mhm yes physics
    class Physics
    {
        public:
        // Shoots a ray from the ray origin in the ray direction and returns a raycasthit
        static RaycastHit Raycast(Ray ray, RaycastHit& rayCastHit);
    };
}

