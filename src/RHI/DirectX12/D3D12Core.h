#pragma once

#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Core {
    bool Initialize();
    void Shutdown();
    //void Update();
    void Render();
}