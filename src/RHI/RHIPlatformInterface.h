#pragma once

#include "../Platform/Window.h"
#include "../Renderer/Renderer.h"

namespace RHI {

    namespace
    {
        using Surface = ::Renderer::Surface;
        using SurfaceID = ::Renderer::SurfaceID;
    }

    struct RHIPlatformInterface
    {
        bool(*Initialize)(void);
        void(*Shutdown)(void);
        //void(*Render)(void);

        struct {
            Surface(*Create)(::Platform::Window);
            void(*Remove)(SurfaceID);
            void(*Resize)(SurfaceID, u32, u32);
            u32(*Width)(SurfaceID);
            u32(*Height)(SurfaceID);
            void(*Render)(SurfaceID);
        } surface;
    };
}

