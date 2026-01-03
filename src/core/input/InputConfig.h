#pragma once

namespace cozy::core
{
    struct InputConfig
    {
        float mouseSensitivity{0.1f};
        float scrollSensitivity{2.0f};

        // We use the integer codes. To keep it decoupled, we don't include GLFW here.
        // These defaults match standard GLFW/ASCII values.
        int keyForward{87};       // W
        int keyBackward{83};      // S
        int keyLeft{65};          // A
        int keyRight{68};         // D
        int keyUp{32};            // SPACE
        int keyDown{341};         // LEFT_CONTROL
        int keyExit{256};         // ESCAPE
        int keyToggleCursor{258}; // TAB

        static constexpr InputConfig Default() noexcept
        {
            return InputConfig{};
        }
    };
}