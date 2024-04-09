#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Parts {
    class D3D12Fence
    {
    public:
        D3D12Fence() = default;
        explicit D3D12Fence(ID3D12Device* const device);
        ~D3D12Fence() = default;

        void WaitForGPU(ID3D12CommandQueue* const cmd_queue);

    public:
#pragma region Getter
        ID3D12Fence1* const GetFence() const { return fence_.Get(); }
        constexpr u64 GetValue() const { return value_; }
        constexpr u32 GetIndex() const { return index_; }
#pragma endregion//Getter
#pragma region Setter
        void SetIndex(const u32 i) { index_ = i; }
#pragma endregion

    private:
        ComPtr<ID3D12Fence1> fence_{ nullptr };
        HANDLE event_{ nullptr };
        u64 value_{ 0 };
        u32 index_{ 0 };
    };
}