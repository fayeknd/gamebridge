#pragma once
#include "../../include/fm/fmath.hpp"

namespace Engine {
    struct Vertex {
        fm::vec3 position;
        fm::vec4 colour;
        fm::vec2 uv;
        fm::vec3 normal;
    };

    constexpr size_t VERTEX_SIZE = sizeof(Vertex);
}