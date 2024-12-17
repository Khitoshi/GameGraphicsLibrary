#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Parts {
    class D3D12Device
    {
    public:
        D3D12Device() = default;
        explicit D3D12Device(IDXGIFactory7* const factory);
        ~D3D12Device() = default;

    public:
#pragma region Getter
        ID3D12Device8* const GetDevice() const { return device_.Get(); }
#pragma endregion//Getter

    private:
        ComPtr<ID3D12Device8> device_{ nullptr };
    };
}
