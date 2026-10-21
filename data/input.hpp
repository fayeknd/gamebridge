#pragma once
#include <keys.hpp>

namespace Engine {

    #define _INTERNAL_KEY_COUNT 256

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
        static inline bool s_keyHeld[keyCount] = {};
        static inline bool s_keyPressed[keyCount] = {};
        static inline bool s_keyReleased[keyCount] = {};

        static void _key_callback(GLFWwindow* wnd, int code, int scancode, int action, int mods);
        static void _mouse_button_callback(GLFWwindow* wnd, int code, int action, int mods);

    public:

        // to convert from key to int we have to cast.
        // to convert from int to key the cast is automatic
        // so we may as well just pass an int to the functions
        // and avoid doing a cast. 
        // this also means we can just pass a MouseBtn enum or
        // a Key enum or anything else

        // Initialize the window for the input class
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

        // Resets the key pressed and released states for the new frame
        static void poll();
    };  
}