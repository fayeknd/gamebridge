#include <GLFW/glfw3.h>
#include <input.hpp>
#include <debug.hpp>
#include <logger.hpp>

using namespace Engine;
using namespace Engine::Debug;

void Input::setMouseHidden(bool v) {

    if (v) glfwSetInputMode(s_wnd, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else   glfwSetInputMode(s_wnd, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    s_mouseHidden = v;
}

// set the mouse position
void Input::setMousePos(fm::dvec2 pos) {
    glfwSetCursorPos(s_wnd, s_mousePos.x, s_mousePos.y);
}

fm::dvec2 Input::getMouseDelta() {
    return s_mouseDelta;
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
    static double lastX = x;
    static double lastY = y;

    double deltaX = x - lastX;
    double deltaY = y - lastY; 

    lastX = x;
    lastY = y;

    s_accumulatedMouseDelta += {deltaX, deltaY};
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

    // we need sub-pixel data
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(wnd, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

void Input::pollStart() {
    if (!isWindowFocused()) {
        s_mouseDelta              = fm::dvec2::zero();
        s_accumulatedMouseDelta   = fm::dvec2::zero();
        return;
    }
    s_mouseDelta = s_accumulatedMouseDelta;
    s_accumulatedMouseDelta = fm::dvec2::zero(); 
}

void Input::pollEnd() {
    s_mousePosLC = s_mousePos;
    s_mouseEnteredTF = false;
    s_mouseExitedTF = false;
    for (size_t i = 0; i < keyCount; i++) {
        s_keyPressed[i] = false;
        s_keyReleased[i] = false;
    }
}