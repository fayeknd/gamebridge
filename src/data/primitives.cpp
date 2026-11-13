#include "gl_context.hpp"
#include "mesh.hpp"
#include <primitives.hpp>

using namespace Engine;

_ModelPtr Primitives::getPrimitive(Primitive p) {

    switch (p) {
        case Primitive::Cube:          return cube();     break;
        case Primitive::InvertedPlane: return invPlane(); break;
        default:                       return plane();    break;
    }

    // key was not valid
    log().logInfo("Primitive key '{}' is not valid key!", static_cast<int>(p));
    return nullptr;
}

_ModelPtr Primitives::plane() {
    static _ModelPtr plane = _generatePlaneImpl();
    return plane;
}

_ModelPtr Primitives::invPlane() {
    static _ModelPtr plane = _generateInvPlaneImpl();
    return plane;
}

_ModelPtr Primitives::cube() {
    static _ModelPtr cube = _generateCubeImpl();
    return cube;
}

_ModelPtr Primitives::_generatePlaneImpl() {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    vertices = {
        // Position                Colour       UV              Normal
        {fm::vec3(-1.0f, 1.0f, 0), fm::vec4(1), fm::vec2(0, 1), fm::vec3::zAxis()},
        {fm::vec3( 1.0f, 1.0f, 0), fm::vec4(1), fm::vec2(1, 1), fm::vec3::zAxis()},
        {fm::vec3( 1.0f,-1.0f, 0), fm::vec4(1), fm::vec2(1, 0), fm::vec3::zAxis()},
        {fm::vec3(-1.0f,-1.0f, 0), fm::vec4(1), fm::vec2(0, 0), fm::vec3::zAxis()},
    };

    indices = {
        0, 1, 3,
        1, 2, 3 
    };
    return Model::createModelWithData(vertices, indices);
}

_ModelPtr Primitives::_generateInvPlaneImpl() {
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    vertices = {
        // Position                Colour       UV              Normal
        {fm::vec3(-1.0f, 1.0f, 0), fm::vec4(1), fm::vec2(0, 1), fm::vec3::zAxis()},
        {fm::vec3( 1.0f, 1.0f, 0), fm::vec4(1), fm::vec2(1, 1), fm::vec3::zAxis()},
        {fm::vec3( 1.0f,-1.0f, 0), fm::vec4(1), fm::vec2(1, 0), fm::vec3::zAxis()},
        {fm::vec3(-1.0f,-1.0f, 0), fm::vec4(1), fm::vec2(0, 0), fm::vec3::zAxis()},
    };

    indices = {
        0, 3, 1,
        1, 3, 2 
    };
    return Model::createModelWithData(vertices, indices);
}

_ModelPtr Primitives::_generateCubeImpl() {

    // making 6 points and creating face data with indices works,
    // but it will not work for UV coordinates.
    // to allow UV coordinates to work, we need to make each 
    // face manually.
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    vertices = {
        // Position                   Colour       UV              Normal
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::zAxis()},
        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::zAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::zAxis()},
        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::zAxis()},
        
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::zAxis()},
        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::zAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::zAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::zAxis()},

        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::yAxis()},
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::yAxis()},
        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::yAxis()},
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::yAxis()},

        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::yAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::yAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::yAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::yAxis()},

        {fm::vec3( 0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 0),-fm::vec3::xAxis()},
        {fm::vec3( 0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 0),-fm::vec3::xAxis()},
        {fm::vec3( 0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(0, 1),-fm::vec3::xAxis()},
        {fm::vec3( 0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(1, 1),-fm::vec3::xAxis()},

        {fm::vec3(-0.5f, 0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 0), fm::vec3::xAxis()},
        {fm::vec3(-0.5f, 0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 0), fm::vec3::xAxis()},
        {fm::vec3(-0.5f,-0.5f, 0.5f), fm::vec4(1), fm::vec2(0, 1), fm::vec3::xAxis()},
        {fm::vec3(-0.5f,-0.5f,-0.5f), fm::vec4(1), fm::vec2(1, 1), fm::vec3::xAxis()}
    };

    indices = {
        0, 1, 3,  
        1, 2, 3,

        4, 5, 7, 
        5, 6, 7,
        
        8, 9, 11, 
        9, 10, 11,

        12, 13, 15,
        13, 14, 15,

        16, 17, 19,
        17, 18, 19,

        20, 21, 23,
        21, 22, 23 
    };

    return Model::createModelWithData(vertices, indices, GL_TRIANGLES);

}