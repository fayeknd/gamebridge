#pragma once
#include <GLFW/glfw3.h> // includes glfw and stuff

namespace Engine {

    // put all the input related stuff in a nested namespace

    // The regular keyboard keys. Trying to include all
    // keys present on most keyboards here based on the
    // glfw3.h file. like these were copied in order
    // from /include/GLFW/glfw3.h
    enum Key {

        UNKNOWN             = -1,

        // printable keys
        Space               = GLFW_KEY_SPACE,
        Apostrophe          = GLFW_KEY_APOSTROPHE,
        Comma               = GLFW_KEY_COMMA,
        Minus               = GLFW_KEY_MINUS,
        Period              = GLFW_KEY_PERIOD,
        FullStop            = Period, // might wanna call it this?
        Slash               = GLFW_KEY_SLASH,
        Backslash           = GLFW_KEY_BACKSLASH,
        Zero                = GLFW_KEY_0,
        One                 = GLFW_KEY_1,
        Two                 = GLFW_KEY_2,
        Three               = GLFW_KEY_3,
        Four                = GLFW_KEY_4,
        Five                = GLFW_KEY_5,
        Six                 = GLFW_KEY_6,
        Seven               = GLFW_KEY_7,
        Eight               = GLFW_KEY_8,
        Nine                = GLFW_KEY_9,
        Semicolon           = GLFW_KEY_SEMICOLON,
        Equals              = GLFW_KEY_EQUAL,
        LeftBracket         = GLFW_KEY_LEFT_BRACKET,
        RightBracket        = GLFW_KEY_RIGHT_BRACKET,
        GraveAccent         = GLFW_KEY_GRAVE_ACCENT, // this key -> `
        Tilde               = GLFW_KEY_GRAVE_ACCENT, // separate definition
        A                   = GLFW_KEY_A,
        B                   = GLFW_KEY_B,
        C                   = GLFW_KEY_C,
        D                   = GLFW_KEY_D,
        E                   = GLFW_KEY_E,
        F                   = GLFW_KEY_F,
        G                   = GLFW_KEY_G,
        H                   = GLFW_KEY_H,
        I                   = GLFW_KEY_I,
        J                   = GLFW_KEY_J,
        K                   = GLFW_KEY_K,
        L                   = GLFW_KEY_L,
        M                   = GLFW_KEY_M,
        N                   = GLFW_KEY_N,
        O                   = GLFW_KEY_O,
        P                   = GLFW_KEY_P,
        Q                   = GLFW_KEY_Q,
        R                   = GLFW_KEY_R,
        S                   = GLFW_KEY_S,
        T                   = GLFW_KEY_T,
        U                   = GLFW_KEY_U,
        V                   = GLFW_KEY_V,
        W                   = GLFW_KEY_W,
        X                   = GLFW_KEY_X,
        Y                   = GLFW_KEY_Y,
        Z                   = GLFW_KEY_Z,
        Placeholder1        = GLFW_KEY_WORLD_1,
        Placeholder2        = GLFW_KEY_WORLD_2,

        // function keys
        Escape              = GLFW_KEY_ESCAPE,
        Enter               = GLFW_KEY_ENTER,
        Tab                 = GLFW_KEY_TAB,
        Backspace           = GLFW_KEY_BACKSPACE,
        Insert              = GLFW_KEY_INSERT,
        Delete              = GLFW_KEY_DELETE,
        Right               = GLFW_KEY_RIGHT,
        Left                = GLFW_KEY_LEFT,
        Down                = GLFW_KEY_DOWN,
        Up                  = GLFW_KEY_UP,
        PageUp              = GLFW_KEY_PAGE_UP,
        PageDown            = GLFW_KEY_PAGE_DOWN,
        Home                = GLFW_KEY_HOME,
        End                 = GLFW_KEY_END,
        CapsLock            = GLFW_KEY_CAPS_LOCK,
        ScrollLock          = GLFW_KEY_SCROLL_LOCK,
        NumLock             = GLFW_KEY_NUM_LOCK,
        PrintScreen         = GLFW_KEY_PRINT_SCREEN,
        Pause               = GLFW_KEY_PAUSE,
        F1                  = GLFW_KEY_F1,
        F2                  = GLFW_KEY_F2,
        F3                  = GLFW_KEY_F3,
        F4                  = GLFW_KEY_F4,
        F5                  = GLFW_KEY_F5,
        F6                  = GLFW_KEY_F6,
        F7                  = GLFW_KEY_F7,
        F8                  = GLFW_KEY_F8,
        F9                  = GLFW_KEY_F9,
        F10                 = GLFW_KEY_F10,
        F11                 = GLFW_KEY_F11,
        F12                 = GLFW_KEY_F12,
        F13                 = GLFW_KEY_F13,  // there are more F keys than just 12, they just aren't on most keyboards.
        F14                 = GLFW_KEY_F14,
        F15                 = GLFW_KEY_F15,
        F16                 = GLFW_KEY_F16,
        F17                 = GLFW_KEY_F17,
        F18                 = GLFW_KEY_F18,
        F19                 = GLFW_KEY_F19,
        F20                 = GLFW_KEY_F20,
        F21                 = GLFW_KEY_F21,
        F22                 = GLFW_KEY_F22,
        F23                 = GLFW_KEY_F23,
        F24                 = GLFW_KEY_F24,
        F25                 = GLFW_KEY_F25,

        // keypad
        KeypadZero          = GLFW_KEY_KP_0,
        KeypadOne           = GLFW_KEY_KP_1,
        KeypadTwo           = GLFW_KEY_KP_2,
        keypadThree         = GLFW_KEY_KP_3,
        KeypadFour          = GLFW_KEY_KP_4,
        KeypadFive          = GLFW_KEY_KP_5,
        KeypadSix           = GLFW_KEY_KP_6,
        KeypadSeveo         = GLFW_KEY_KP_7,
        KeypadEight         = GLFW_KEY_KP_8,
        KeypadNine          = GLFW_KEY_KP_9,
        KeypadDecimal       = GLFW_KEY_KP_DECIMAL,
        KeypadDivide        = GLFW_KEY_KP_DIVIDE,
        KeypadMultiply      = GLFW_KEY_KP_MULTIPLY,
        KeypadSubtract      = GLFW_KEY_KP_SUBTRACT,
        KeypadAdd           = GLFW_KEY_KP_ADD,
        KeypadEnter         = GLFW_KEY_KP_ENTER,
        KeypadEquals        = GLFW_KEY_KP_EQUAL,

        // idk what to call these keys but they need a line separator cause theyre different
        LeftShift           = GLFW_KEY_LEFT_SHIFT,
        LeftControl         = GLFW_KEY_LEFT_CONTROL,
        LeftAlt             = GLFW_KEY_LEFT_ALT,
        LeftSuper           = GLFW_KEY_LEFT_SUPER,
        RightShift          = GLFW_KEY_RIGHT_SHIFT,
        RightControl        = GLFW_KEY_RIGHT_CONTROL,
        RightAlt            = GLFW_KEY_RIGHT_ALT,
        RightSuper          = GLFW_KEY_RIGHT_SUPER,
        RightMenu           = GLFW_KEY_MENU

    };


    enum MouseBtn {

        B1                  = GLFW_MOUSE_BUTTON_1,
        B2                  = GLFW_MOUSE_BUTTON_2,
        B3                  = GLFW_MOUSE_BUTTON_3,
        B4                  = GLFW_MOUSE_BUTTON_4,
        B5                  = GLFW_MOUSE_BUTTON_5,
        B6                  = GLFW_MOUSE_BUTTON_6,
        B7                  = GLFW_MOUSE_BUTTON_7,
        B8                  = GLFW_MOUSE_BUTTON_8,

        // better names
        LeftMouse           = B1, 
        RightMouse          = B2,
        MiddleMouse         = B3

    };


    enum GamepadBtn {

        // xbox/nintendo/anything else defs
        PadA                = GLFW_GAMEPAD_BUTTON_A,
        PadB                = GLFW_GAMEPAD_BUTTON_B,
        PadX                = GLFW_GAMEPAD_BUTTON_X,
        PadY                = GLFW_GAMEPAD_BUTTON_Y,
        // playstation defs
        PadCross            = GLFW_GAMEPAD_BUTTON_CROSS,
        PadCircle           = GLFW_GAMEPAD_BUTTON_CIRCLE,
        PadSquare           = GLFW_GAMEPAD_BUTTON_SQUARE,
        PadTriangle         = GLFW_GAMEPAD_BUTTON_TRIANGLE,

        DPadUP              = GLFW_GAMEPAD_BUTTON_DPAD_UP,
        DPadRight           = GLFW_GAMEPAD_BUTTON_DPAD_RIGHT,
        DPadDown            = GLFW_GAMEPAD_BUTTON_DPAD_DOWN,
        DPadLeft            = GLFW_GAMEPAD_BUTTON_DPAD_LEFT,

        PadLB               = GLFW_GAMEPAD_BUTTON_LEFT_BUMPER,
        PadRB               = GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER,
        PadLT               = GLFW_GAMEPAD_AXIS_LEFT_TRIGGER,
        PatRT               = GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER,
        PadBack             = GLFW_GAMEPAD_BUTTON_BACK,
        PadStart            = GLFW_GAMEPAD_BUTTON_START,
        PadGuide            = GLFW_GAMEPAD_BUTTON_GUIDE,
        PadLeftThumbstick   = GLFW_GAMEPAD_BUTTON_LEFT_THUMB,
        PadRightThumbstick  = GLFW_GAMEPAD_BUTTON_RIGHT_THUMB,
        PadLeftStickX       = GLFW_GAMEPAD_AXIS_LEFT_X,
        PadLeftStickY       = GLFW_GAMEPAD_AXIS_LEFT_Y,
        PadRightStickX      = GLFW_GAMEPAD_AXIS_RIGHT_X,
        PadRightStickY      = GLFW_GAMEPAD_AXIS_RIGHT_Y,
    
    };
}
