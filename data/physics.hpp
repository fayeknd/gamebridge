#pragma once
#include <fm/fmath.hpp>
#include <entity.hpp>
#include <ray.hpp>

namespace Engine {

    // moved ray stuff to ray.hpp.
    // made Physics a namespace so multiple files can contribute to it.

    // Mhm yes physics
    namespace Physics {

        // Shoots a ray from the ray origin in the ray direction and returns a raycasthit
        RaycastHit Raycast(Ray ray, RaycastHit& rayCastHit, float distance = physics_defaultRayDistance);
    };
}

