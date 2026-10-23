#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <engine.h>
#include <renderdata.hpp>
#include <logger.hpp>
#include <queue>

namespace Engine {

    // the GLContext is a singleton responsible for wrapping the OpenGL context neatly.
    // the OpenGL context is simply just the "opengl instance" for this program. 
    // effectively this class just informs us whether the context is ready for use or not.
    // it also has the GLState, which is overviewed further down.

    class GLContext {
    protected:

        // determines if opengl is ready to use. returned by isGLReady().
        bool m_glInit = false;

        // maximum allowed bound textures
        uint8_t m_maxTextureSlots;

        friend class Window;
        
    public:

        // example of a static instance singleton. having one of these in a class
        // effectively means it is a singleton, or static class. it can be used via Class::get().___
        // this means you don't need to add static to the start of each function. it also makes it
        // clearer what is and isn't a singleton.

        static Logging::Logger& log() {
            static Logging::Logger instance("OPENGL");
            return instance;
        }

        static GLContext& get() {

            // static variables defined in a function are only defined once. whenever get() is called, 
            // the same "instance" variable will be returned and not recreated from scratch.
            static GLContext instance;
            return instance;
        }

        // self-explanitory, returns true if the glcontext is usable and false if it is not usable.
        bool& isGLReady();

        // creates the gl glfw context. will only create the glfw context if m_glInit is false.
        // window hints and extra window hints are lambda functions passed in and called inside the function.
        // window_hints is the function for the default glfw window hints (i.e GLFW_TRANSPARENT_FRAMEBUFFER).
        // these are set in window.hpp.

        // extra_window_hints is an additional optional stage for setting any extra alternative window hints. not required.
        int createGLFWContext(void(*window_hints)(), void(*extra_window_hints)() = nullptr);

        // this one is annoying. the GLAD context is actually responsible for creating the GL context and ideally would have
        // been created with the createGLFWContext function, combining into the createGLContext function.
        // unfortunately, the glad context HAS to be made after the main window.
        int createGLADContext();

        // destroys the gl(fw) context. again, will only destroy the context if m_glInit is true.
        // this is called by the destructor, so there isn't really a need to call this yourself
        // unless you're doing something really specific. 
        int destroyGLContext();

        uint8_t maxTextureSlots() { return m_maxTextureSlots; }

        // destructor just calls destroyGLContext().
        ~GLContext();

        // the GLState is responsible for efficiently tracking different
        // bound objects without the use of glGetIntegeriv.
        // (afaik glGetIntegeriv isn't very efficient.)
        // can be aquired via GLContext::get().glState.

        // once again, this should not really be accessed in any 
        // scripts or anything unless you're doing something really specific. 

        // this will be added to as the program grows and more opengl buffers and stuff are included.

        struct GLState {

            // the currently bound shader program
            GLuint m_currentProgram = -1;

            // the currently bound vertex array
            GLuint m_currentMesh = -1; 

            // the texture queue. up to 32 textures can be
            // bound at any time, so we store them in a queue.
            std::queue<GLuint> m_txQueue;

            // the active unit.
            GLuint m_activeTextureUnit = -1;

        } glState;

    };

}