#include "gl_context.hpp"
#include "mesh.hpp"
#include <primitives.hpp>

using namespace Engine;

std::shared_ptr<Mesh> Primitives::getPrimitive(Primitive p) {

    switch (p) {
        case Primitive::Cube: {
            return cube();
            break;
        }
    }

    // key was not valid
    log().logInfo("Primitive key '{}' is not valid key!");
    return nullptr;
}

std::shared_ptr<Mesh> Primitives::cube() {
    static std::shared_ptr<Mesh> cube = _generateCubeImpl();
    return cube;
}

std::shared_ptr<Mesh> Primitives::_generateCubeImpl() {


    // making 6 points and creating face data with indices works,
    // but it will not work for UV coordinates.
    // to allow UV coordinates to work, we need to make each 
    // face manually.
    std::vector<Vertex> vertices;
    std::vector<GLushort> indices;
    vertices = {
        // Position                   Colour       UV              Normal
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::zAxis()},
        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::zAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::zAxis()},
        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::zAxis()},
        
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::zAxis()},
        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::zAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::zAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::zAxis()},

        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::yAxis()},
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::yAxis()},
        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::yAxis()},
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::yAxis()},

        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::yAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::yAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::yAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::yAxis()},

        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::xAxis()},
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::xAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::xAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::xAxis()},

        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::xAxis()},
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::xAxis()},
        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::xAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::xAxis()}
    };

    indices = {
        0, 1, 2,  
        0, 2, 3,

        4, 5, 6, 
        4, 6, 7,
        
        8, 9, 10, 
        8, 10, 11,

        12, 13, 14,
        12, 14, 15,

        16, 17, 18,
        16, 18, 19,

        20, 21, 22,
        20, 22, 23 
    };

    return Mesh::createMeshWithData(vertices, indices, GL_TRIANGLES);

}