#pragma once
#include <mesh.hpp>

namespace Engine {

    // was getting annoyed tpying this so much
    using _MeshPtr = std::shared_ptr<Mesh>;

    // primitive enum is in mesh.hpp 
    // i had a reason to do this but i forgot 

    // a collection of primitives
    class Primitives {
    protected:

        static _MeshPtr _generateCubeImpl();
        static _MeshPtr _generatePlaneImpl();

        static Logging::Logger& log() {
            static Logging::Logger instance("PRIMITIVE");
            return instance;
        }

    public:

        // no unordered_map or anything, there are going to be 
        // a constant amount of primitives that we control
        static _MeshPtr getPrimitive(Primitive p);

        static _MeshPtr cube();
        static _MeshPtr plane();

    };
}