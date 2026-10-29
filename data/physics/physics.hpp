#pragma once
#include <fm/fmath.hpp>
#include <entity.hpp>
#include <physics/ray.hpp>
#include <time.hpp>
#include <layer.hpp>

namespace Engine {

    // moved ray stuff to ray.hpp.
    // made Physics a namespace so multiple files can contribute to it.

    // Mhm yes physics
    namespace Physics {

        // Shoots a ray from the ray origin in the ray direction and returns a raycasthit
        RaycastHit Raycast(Ray ray, LayerMask layers = LayerMasks::All, float distance = physics_defaultRayDistance);
    };

    // Controls the physics ticks system
    class PhysicsImpl {
    private:

        static void pollPhysics();

        friend class Renderer;

    public:

        // moved here, because static variables in a namespace
        // will create a new instance for each thing that includes
        // the file.
        // fucking. stupid but. whatever. - faye

        // Physics tick, 20 ticks per second
        static inline Timer physicsTick;
        // Ticks per second
        static inline uint16_t tps = 60;

    };
}