#pragma once
#include "GLFW/glfw3.h"
#include <keys.hpp>
#include <fm/fmath.hpp>

namespace Engine {

    #define _INTERNAL_KEY_COUNT 298

    // Input Class, handles input by storing the flag state of every key in the game and allows
    // for input to be handled by GetKeyPressed, GetKeyDown & GetKeyUp

    // m_ makes it clear it's a member/instance variable and s_ makes it clear it is static
    // just a convention we gotta keep consistent
    class Input {
    private:
    // Amount of keys that the current system stores
        static constexpr int keyCount = static_cast<int>(_INTERNAL_KEY_COUNT);

        // Booleans to each key returning whether the key is held, pressed or released on
        // the last frame
        static inline bool s_keyHeld[keyCount]     = { };
        static inline bool s_keyPressed[keyCount]  = { };
        static inline bool s_keyReleased[keyCount] = { };

        static void _key_callback(GLFWwindow* wnd, int code, int scancode, int action, int mods);
        static void _mouse_button_callback(GLFWwindow* wnd, int code, int action, int mods);
        static void _mouse_pos_callback(GLFWwindow* wnd, double x, double y);
        static void _enter_Renderer_callback(GLFWwindow* wnd, int state);
        static void _window_focus_callback(GLFWwindow* wnd, int state);

        // has the mouse entered the Renderer this frame?
        static inline bool s_mouseEnteredTF;
        // has the mouse left the Renderer this frame?
        static inline bool s_mouseExitedTF;

        // mouse pos last callback (as double to match glfw)
        static inline fm::dvec2 s_mousePosLC            = {0};
        // mouse pos this callback    
        static inline fm::dvec2 s_mousePos              = {0}; 
        
        // accumulate mouse delta over time. 
        // fixes some high fps issues.
        static inline fm::dvec2 s_accumulatedMouseDelta = {0};
        // the cached mouse delta.
        static inline fm::dvec2 s_mouseDelta            = {0};

        // is the Renderer focused? 
        static inline bool s_RendererFocused = true;
        
        // is the mouse hidden?
        static inline bool s_mouseHidden = false;

        static inline GLFWwindow* s_wnd = nullptr; // store a ref to the Renderer.
        // don't use Renderer.hpp because that can cause issues. i hate this solution.

        
    public:

        static inline bool getMouseEnteredRenderer() { return s_mouseEnteredTF; }
        static inline bool getMouseExitedRenderer() { return s_mouseEnteredTF; }
        static inline bool isRendererFocused() { return s_RendererFocused; }
        // get the mouse pos
        static inline fm::dvec2 getMousePos() { return s_mousePos; }
        static inline fm::dvec2 getMousePosLastCallback() { return s_mousePosLC; } // if you want this? idk what you'd want it for
        static inline bool isMouseHidden() { return s_mouseHidden; }
        
        static void setMouseHidden(bool v);
        static void setMousePos(fm::dvec2 pos);

        // get the amount the mouse moved this frame
        static fm::dvec2 getMouseDelta();

        // to convert from key to int we have to cast.
        // to convert from int to key the cast is automatic
        // so we may as well just pass an int to the functions
        // and avoid doing a cast. 
        // this also means we can just pass a MouseBtn enum or
        // a Key enum or anything else

        // Initialize the Renderer for the input class
        static void init(GLFWwindow* wnd);

        // Sets the key down state assuming key isnt held otherwise the key is held down
        static void onActionDown(int key);

        // Sets the key up state by setting key held state to false and released to true
        static void onActionUp(int key);

        // Returns the state of if the key held
        static bool getAction(int key)     { return s_keyHeld[key]; }
        // Returns the state of if the key is pressed
        static bool getActionDown(int key) { return s_keyPressed[key]; }
        // Returns the state of if the key is released
        static bool getActionUp(int key)   { return s_keyReleased[key]; }
        
        // update values at the end of the frame.
        static void pollEnd();
        // update values at the start of the frame.
        static void pollStart(); 
    };  
}