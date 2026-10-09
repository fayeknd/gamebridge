#pragma once
#include "../../include/fm/fmath.hpp"

namespace Engine {

    // just a simple struct for vertex information.

    struct Vertex {

        // a vec3. x-y-z.
        fm::vec3 position;

        // a vec4. r-g-b-a. 
        fm::vec4 colour;

        // a vec2. u-v.
        fm::vec2 uv;

        // a normalized vec3. just a direction.
        fm::vec3 normal;
    };
}