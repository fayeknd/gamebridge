#include <input.hpp>

using namespace Engine;

// Sets the key down state assuming key isnt held otherwise the key is held down
void Input::onActionDown(int key) {
    if (!s_keyHeld[key]) 
        s_keyPressed[key] = true;
    s_keyHeld[key] = true;
}

// Sets the key up state by setting key held state to false and released to true
void Input::onActionUp(int key) {
    s_keyHeld[key] = false;
    s_keyReleased[key] = true;
}

//Sets the key down and key up states using GLFW's actions
void Input::_key_callback(GLFWwindow*, int code, int, int action, int) {
    if (action == GLFW_PRESS)        onActionDown(code);
    else if (action == GLFW_RELEASE) onActionUp(code);
}

// Sets the key down and key up states with GLFW's mouse button actions
void Input::_mouse_button_callback(GLFWwindow*, int code, int action, int) {
    if (action == GLFW_PRESS)        onActionDown(code);
    else if (action == GLFW_RELEASE) onActionUp(code);
}

// Sets the keycallback context and mouse button context
void Input::init(GLFWwindow* wnd) {
    glfwSetKeyCallback(wnd, _key_callback);
    glfwSetMouseButtonCallback(wnd, _mouse_button_callback);
}

void Input::poll() {
    for (size_t i = 0; i < keyCount; i++) {
        s_keyPressed[i] = false;
        s_keyReleased[i] = false;
    }
}