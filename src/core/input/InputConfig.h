#pragma once

namespace cozy::core
{
    struct InputConfig
    {
        float mouseSensitivity{0.1f};
        float scrollSensitivity{2.0f};

        int keyForward{87};       // W
        int keyBackward{83};      // S
        int keyLeft{65};          // A
        int keyRight{68};         // D
        int keyUp{32};            // SPACE
        int keyDown{341};         // LEFT_CONTROL
        int keyExit{256};         // ESCAPE
        int keyToggleCursor{258}; // TAB

        int keySprint{340}; // LEFT_SHIFT
        int keyZoomIn{81};  // Q (Optional alternative to scroll)
        int keyZoomOut{69}; // E (Optional alternative to scroll)

        static constexpr InputConfig Default() noexcept
        {
            return InputConfig{};
        }
    };
}