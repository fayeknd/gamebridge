#pragma once
#include <GLFW/glfw3.h>

namespace Engine
{

    // Every key that's going to be used in the game
    enum class Key
    {
        W, A, S, D, LMB, 
        
        // Amount of Keys
        Count
    };

    // Input Class, handles input by storing the flag state of every key in the game and allows
    // for input to be handled by GetKeyPressed, GetKeyDown & GetKeyUp
    class Input
    {
       private:
       // Amount of keys that the current system stores
        static constexpr int keyCount = static_cast<int>(Key::Count);

        // Booleans to each key returning whether the key is held, pressed or released on
        // the last frame
        static inline bool keyHeld[keyCount] = {};
        static inline bool keyPressed[keyCount] = {};
        static inline bool keyReleased[keyCount] = {};

        static void KeyCallback(GLFWwindow* wnd, int glfwKey, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* wnd, int glfwButton, int action, int mods);

        static bool TranslateKey(int glfwKey, Key& out);
        static bool TranslateMouseButton(int glfwButton, Key& out);

       public:
       // Initialize the window for the input class
       static void Init(GLFWwindow* wnd);

       // Sets the key down state assuming key isnt held otherwise the key is held down
        static void OnKeyDown(Key key)
        {
            int i = static_cast<int>(key);
            if (!keyHeld[i])
             keyPressed[i] = true;
            keyHeld[i] = true;
        }   

        // Sets the key up state by setting key held state to false and released to true
        static void OnKeyUp(Key key)
        {
            int i = static_cast<int>(key);
            keyHeld[i] = false;
            keyReleased[i] = true;
        }

        // Returns the state of if the key held
        static bool GetKey(Key key)     { return keyHeld[static_cast<int>(key)]; }
        // Returns the state of if the key is pressed
        static bool GetKeyDown(Key key) { return keyPressed[static_cast<int>(key)]; }
        // Returns the state of if the key is released
        static bool GetKeyUp(Key key)   { return keyReleased[static_cast<int>(key)]; }

        // Resets the key pressed and released states for the new frame
        static void NewFrame()
        {
            for (int i = 0; i < keyCount; i++)
            {
                keyPressed[i] = false;
                keyReleased[i] = false;
            }
        }
    };  

}