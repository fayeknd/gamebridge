#pragma once
#include <fm/fmath.hpp>
#include <limits>

namespace Engine {

    constexpr float physics_defaultRayDistance = std::numeric_limits<float>::max();

    // cheeky forward declaration
    class BoundingBox;
    class Entity;

    namespace Physics {

        // Data for what the raycast hit, whether the value was a hit, the distance of the hit, where the hit was on the collider
        // and the normal of the hit

        struct RaycastHitInfo {
            bool m_hasHit            = false;
            float m_distance         = physics_defaultRayDistance;
            fm::vec3 m_hitPoint      = {};
            fm::vec3 m_farHitPoint   = {};
            fm::vec3 m_normal        = {};
        };

        struct RaycastHit {
            RaycastHitInfo m_hitInfo = {};
            Entity* m_hitEntity      = nullptr; // c++ doesn't auto initialise variables, so we should do it
            BoundingBox* m_hitBox    = nullptr; // so we don't have to needlessly do getComponent<BoundingBox>

            RaycastHit() = default;

            explicit operator bool() const noexcept {
                return (m_hitEntity != nullptr);
            }
        };

        struct Ray
        {
            fm::vec3 m_origin        = {};
            fm::vec3 m_direction     = {};
        };
    }
}