#pragma once
#include <mesh.hpp>

namespace Engine {

    // primitive enum is in mesh.hpp 

    // a collection of primitives
    class Primitives {
    protected:

        static std::shared_ptr<Mesh> _generateCubeImpl();

        static Logging::Logger& log() {
            static Logging::Logger instance("PRIMITIVE");
            return instance;
        }

    public:

        // no unordered_map or anything, there are going to be 
        // a constant amount of primitives that we control
        static std::shared_ptr<Mesh> getPrimitive(Primitive p);

        static std::shared_ptr<Mesh> cube();

    };
}