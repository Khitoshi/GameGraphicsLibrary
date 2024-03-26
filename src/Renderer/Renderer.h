#pragma once

#include "CommonHeaders.h"
#include "..\Platform\Window.h"

namespace Renderer {

    class surface
    {};

    struct RenderSurface
    {
        Platform::Window window{};
        surface surface{};
    };

    enum class GRAPHICS_PLATFORM :u32
    {
        direct3d12 = 0,
    };

    bool Initialize(GRAPHICS_PLATFORM platform);
    void Shutdown();

}