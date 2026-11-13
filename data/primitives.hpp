#pragma once
#include <model.hpp>

namespace Engine {

    // was getting annoyed tpying this so much
    using _ModelPtr = std::shared_ptr<Model>;

    // primitive enum is in model.hpp 
    // i had a reason to do this but i forgot 

    // a collection of primitives
    class Primitives {
    protected:

        static _ModelPtr _generateCubeImpl();
        static _ModelPtr _generatePlaneImpl();
        static _ModelPtr _generateInvPlaneImpl();

        static Logging::Logger& log() {
            static Logging::Logger instance("PRIMITIVE");
            return instance;
        }

    public:

        // no unordered_map or anything, there are going to be 
        // a constant amount of primitives that we control
        static _ModelPtr getPrimitive(Primitive p);

        static _ModelPtr cube();
        static _ModelPtr plane();
        static _ModelPtr invPlane();

    };
}