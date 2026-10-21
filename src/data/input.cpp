#include <input.hpp>

using namespace Engine;

// Converts GLFW keys to my key enumerator
bool Input::TranslateKey(int glfwKey, Key& out)
{
    switch (glfwKey)
    {
        case GLFW_KEY_W: out = Key::W; return true;
        case GLFW_KEY_A: out = Key::A; return true;
        case GLFW_KEY_S: out = Key::S; return true;
        case GLFW_KEY_D: out = Key::D; return true;
        default:         return false;   // anything else
    }
}

// Translates GLFW mouse buttons to my key enumerator
bool Input::TranslateMouseButton(int glfwButton, Key& out)
{
    switch (glfwButton)
    {
        case GLFW_MOUSE_BUTTON_LEFT: out = Key::LMB; return true;
        default:                     return false; // anything else
    }
}

//Sets the key down and key up states using GLFW's actions
void Input::KeyCallback(GLFWwindow*, int glfwKey, int, int action, int)
{
    Key key;
    if (!TranslateKey(glfwKey, key)) return; // prevents multiple triggers on the same frame

    if (action == GLFW_PRESS)        OnKeyDown(key);
    else if (action == GLFW_RELEASE) OnKeyUp(key);
}

// Sets the key down and key up states with GLFW's mouse button actions
void Input::MouseButtonCallback(GLFWwindow*, int glfwButton, int action, int)
{
    Key key;
    if (!TranslateMouseButton(glfwButton, key)) return; // prevents multiple triggers on the same frame

    if (action == GLFW_PRESS)        OnKeyDown(key);
    else if (action == GLFW_RELEASE) OnKeyUp(key);
}

// Sets the keycallback context and mouse button context
void Input::Init(GLFWwindow* wnd)
{
    glfwSetKeyCallback(wnd, KeyCallback);
    glfwSetMouseButtonCallback(wnd, MouseButtonCallback);
}