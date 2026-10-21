#include "GLFW/glfw3.h"
#include <input.hpp>

using namespace Engine;

// set the mouse position
void Input::setMousePos(fm::dvec2 pos) {
    glfwSetCursorPos(s_wnd, s_mousePos.x, s_mousePos.y);
}

fm::dvec2 Input::getMouseDelta() {
    if (!isWindowFocused()) return 0; 
    return s_mousePos - s_mousePosLC;
}

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

void Input::_mouse_pos_callback(GLFWwindow *wnd, double x, double y) {
    static bool first = true;
    s_mousePos = fm::vec(x, y);
    if (first || s_mouseEnteredTF) {
        s_mousePosLC = s_mousePos;
        first = false;
    }
}

// i don't know what the int is for
void Input::_enter_window_callback(GLFWwindow *wnd, int state) {
    if (state) 
        s_mouseEnteredTF = true;
    else
        s_mouseExitedTF = true;
}

void Input::_window_focus_callback(GLFWwindow *wnd, int state) {
    s_windowFocused = state;
}

// Sets the keycallback context and mouse button context
void Input::init(GLFWwindow* wnd) {
    s_wnd = wnd;
    glfwSetKeyCallback(wnd, _key_callback);
    glfwSetMouseButtonCallback(wnd, _mouse_button_callback);
    glfwSetCursorPosCallback(wnd, _mouse_pos_callback);
    glfwSetCursorEnterCallback(wnd, _enter_window_callback);
    glfwSetWindowFocusCallback(wnd, _window_focus_callback);
}

void Input::poll() {
    s_mousePosLC = s_mousePos;
    s_mouseEnteredTF = false;
    s_mouseExitedTF = false;
    for (size_t i = 0; i < keyCount; i++) {
        s_keyPressed[i] = false;
        s_keyReleased[i] = false;
    }
}