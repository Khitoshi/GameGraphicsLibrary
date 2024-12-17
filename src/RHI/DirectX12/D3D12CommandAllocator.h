#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Parts {
    class D3D12CommandAllocator
    {
    public:
        D3D12CommandAllocator() = default;
        explicit D3D12CommandAllocator(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE type);
        ~D3D12CommandAllocator() = default;

        void Reset();

#pragma region Getter
        ID3D12CommandAllocator* const GetCommandAllocator() const { return command_allocator_.Get(); }
#pragma endregion//Getter

    private:
        ComPtr<ID3D12CommandAllocator> command_allocator_{ nullptr };
    };
}
