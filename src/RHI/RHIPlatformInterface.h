#pragma once

namespace RHI {

    struct RHIPlatformInterface
    {
        bool(*Initialize)(void);
        void(*Shutdown)(void);
        void(*Render)(void);
    };
}

