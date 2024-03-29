#pragma once

#include "D3D12CommonHeaders.h"

// ‘O•ûéŒ¾
namespace RHI::D3D12::Parts {
    class DescriptorHeapManager;
}

namespace RHI::D3D12::Core {


    bool Initialize();
    void Shutdown();
    //void Update();
    void Render();

#pragma region Getter
    ID3D12Device* const GetDevice();
    u32 GetCurrentFrameIndex();

    RHI::D3D12::Parts::DescriptorHeapManager* GetDescriptorHeapManager();
#pragma endregion//Getter
}