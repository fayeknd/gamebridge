#pragma once
#include <debug.hpp>
#include <fm/fmath.hpp>
#include <component/boundingbox.hpp>

// TODO : calculate bounds if the entity transform is dirty rather than just when drawing

namespace Engine
{
    // Bounding box data for collisions, anything between minimum and maximum
    class BoundingBox {
    public:
        fm::vec3 m_min =-fm::vec3::one() * 0.5f; // bottom-left-front bounds
        fm::vec3 m_max = fm::vec3::one() * 0.5f; // top-right-back bounds

        // Checks for a collision and returns true if two bounding boxes overlap each other
        void setBounds(fm::vec3 min, fm::vec3 max);
    };
}