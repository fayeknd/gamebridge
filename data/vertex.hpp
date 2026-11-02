#pragma once
#include <fm/fmath.hpp>

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

        Vertex(fm::vec3 p, fm::vec4 c, fm::vec2 u, fm::vec3 n) {
            position = p;
            colour = c;
            uv = u;
            normal = n;
        }

        Vertex(fm::vec3 p) {
            position = p;
            colour = {1};
            uv = {0};
            normal = {0};
        }

        Vertex(fm::vec3 p, fm::vec4 c) {
            position = p;
            colour = c;
            uv = {0};
            normal = {0};
        }

        Vertex() {
            position = {0};
            colour = {1};
            uv = {0};
            normal = {0};
        };

    };

    using Point = Vertex;
}