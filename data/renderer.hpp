#pragma once

#include <gl_context.hpp>
#include <fm/fmath.hpp>
#include <string>
#include <command.hpp>

namespace Engine {

    constexpr size_t renderer_maxLayers = 256;

    // forward declarations
    class Mesh;
    class Material;

    // FullscreenMode is just an enumerator to store different Renderer states.
    // pretty self explanitory.
    enum FullscreenMode {
        Fullscreen              = 0x01,
        Borderless              = 0x02,
        BorderlessRenderered      = 0x03,
        Renderered                = 0x04
    };

    enum VSyncMode {
        On                      = 1,
        Off                     = 0,
        Adaptive                =-1,
    };

    // to keep the program simple, the Renderer class is a singleton.
    // i made this design choice because it's easy to keep track of
    // and it's easy to just get the Renderer and do stuff without
    // having to make sure you're getting the "main window".

    class Renderer {
    protected:

        // this is only used when swapping between fullscreen modes.
        // if in fullscreen, the old pos and size will be cached here
        // so the wuin window position and size can be faithfully restored
        // if the program returns to windowed mode.
        fm::uvec2 m_wndSize_old;
        fm::uvec2 m_wndPos_old; 
        
        // Renderer size. uvec2 because these values can never be negative or decimals.
        // (uvec is just a vector of type unsigned integer. equivalent to fm::_vec2<unsigned int>)
        fm::uvec2 m_wndSize = fm::uvec2(800,800);

        // Renderer title        had to show this to professional people and couldn't have it being called Trans Ahh Game - faye
        std::string m_wndTitle = "gamebridge";
        
        // GLFW window handle. required for a lot of things as you may know.
        // i believe on windows there is a function to get the HWND handle from this.
        GLFWwindow* m_wnd = nullptr;

        // store the current window state as well.
        FullscreenMode m_fsMode = FullscreenMode::Renderered;

        // this function defines what glfw Renderer hints are set by default.
        // i.e glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // this is passed through to the GLContext on window creation as a lambda.
        static void _default_Renderer_hints();

        // handles changing the m_wndSize value and glViewport.
        static void _framebuffer_resize_callback(GLFWwindow* wnd, int w, int h);

        // target frames per second, self explanitory 
        uint16_t m_targetFramesPerSecond;
        uint16_t m_currentFramesPerSecond;

        // target delta time. what is used to limit fps.
        double m_targetFrameDelta;

        // the current vsync mode.
        VSyncMode m_vsyncMode;

        // the current frame.
        uint32_t m_frame;

        // this is a technique called bucket rendering, where we render in chunks.
        // we are not rendering in traditional "minecraft-like" chunks,
        // but rather layered chunks.

        // our render buckets. a big array of commands. 
        std::array<std::vector<RenderCommand>, renderer_maxLayers> m_commandBuckets;

        // we have a maximum of 256 buckets. we will never use all of these.
        // if a bucket has no data, sorting it is a waste of resources.
        std::array<bool, renderer_maxLayers> m_bucketHasData = { false };

    public:

    // TODO : put this somewhere better (or dont)
        bool m_viewportDirty = false;

        // these are setters/getters to ensure that targetFrameDelta and
        // targetFramesPerSecond are representing the same thing.
        // for instance, an fps of 60 means the delta is 0.0166...
        // and a delta of 0.0166... means the fps is 60.

        // get desired fps
        uint16_t getTargetFPS();

        uint16_t getFPS();

        // get desired frame delta
        double getTargetFrameDelta();

        // set desired fps
        void setTargetFPS(uint16_t fps);

        // set desired frame delta
        void setTargetFrameDelta(double delta);

        // get the current vsync mode
        VSyncMode getVSyncMode();

        // get the frame
        uint32_t getFrame() { return m_frame; }

        // set the current vsync mode
        void setVSyncMode(VSyncMode vsm);

        // return a fractional aspect ratio (e.g 16:9)
        fm::uvec2 getFractionalAspectRatio();
        // return a regular float aspect ratio (e.g 1.7777..)
        double getAspectRatio();

        // there's a lot of these dotted around the program for a good reason.
        // check out gl_context.hpp for an explanation of why.
        static Renderer& get() {
            static Renderer instance;
            return instance;
        }

        // returns the Renderer size.
        fm::uvec2 getWndSize();

        // sets the Renderer size via glfwSetRendererSize(m_wnd, v.x, v.y).
        void setWndSize(fm::uvec2 v);
        

        // returns the Renderer title
        std::string getWndTitle();

        // sets the Renderer title via glfwSetRendererTitle(m_wnd, s.c_str()).
        void setWndTitle(std::string s);


        // returns the glfw Renderer handle. (different from the HWND Renderer handle.)
        GLFWwindow* getWndHandle() const;

        // creates the Renderer. this should be the first function to run.
        // a run through of what the parameters defaults are for:
        
        // size: the Renderer size. leave at -1 for the default value of 800x800. 
        // title: Renderer title. -1 for default also.
        // fsMode: fullscreen mode. by default is just Renderered.
        // monitor: leave null for now.
        // share: i don't really know what this does, it just gets passed through to glfwCreateRenderer.
        // extra_Renderer_hints: a lambda function to add your own Renderer hints in. create a function that
        // has GLFWwindowHint() functions and pass that function through as a parameter. not required.
        int createWnd(fm::uvec2 size = fm::uvec2(-1), std::string title = "-1", FullscreenMode fsMode = FullscreenMode::Renderered, GLFWmonitor* monitor = NULL, GLFWwindow* share = NULL, void(*extra_Renderer_hints)() = nullptr);

        // returns the current Renderer fullscreen state.
        FullscreenMode getFullscreenMode();

        // sets the Renderer state via a switch statement. 
        void setFullscreenMode(FullscreenMode fm);

        // begins the program rendering and logic loop. in main(), this should come last unless you have
        // any cleanup stuff to perform after the program stops.
        void beginGameLoop();

        // put anything that polls in these function.
        void preFramePollEvents();
        void postFramePollEvents();

        // clean up the program after exiting.
        void cleanup();

        // actual pipeline stuff

        // add a render command to the render pipeline
        void queueRenderCommand(uint8_t layer, uint64_t sortKey, IComponent* component, void(*draw_callback)(IComponent*));
        void flushRenderCommands();

        // create a sort key for render commands to use
        uint64_t genSortKey(bool transparent, Material* mat, float depth);
        // compress a standard 64-bit pointer down to a 16-bit hash
        uint16_t compressPtrToHash(const void* ptr);
    };
}