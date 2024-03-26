#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Parts {

    class D3D12CommandList
    {
    public:
        D3D12CommandList() = default;
        explicit D3D12CommandList(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* const cmd_allocator);
        ~D3D12CommandList() = default;

        void Reset(ID3D12CommandAllocator* const cmd_allocator);
        void Close();

#pragma region Getter
        ID3D12GraphicsCommandList6* const GetCommandList() const { return command_list_.Get(); }
#pragma endregion// Getter


    private:
        ComPtr<ID3D12GraphicsCommandList6> command_list_{ nullptr };
    };

}
