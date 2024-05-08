#pragma once

#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::PostProcess
{
    bool Initialize();
    void Shutdown();

    void PostProcess(ID3D12GraphicsCommandList6* cmd_list, D3D12_CPU_DESCRIPTOR_HANDLE target_rtv);
}
