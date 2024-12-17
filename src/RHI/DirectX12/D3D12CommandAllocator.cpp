#include "D3D12CommandAllocator.h"

namespace RHI::D3D12::Parts
{
    D3D12CommandAllocator::D3D12CommandAllocator(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE type)
    {
        HRESULT hr{ S_OK };
        hr = device->CreateCommandAllocator(type, IID_PPV_ARGS(&command_allocator_));
        DXCall(hr);
        assert(SUCCEEDED(hr));
        NAME_D3D12_OBJECT(command_allocator_.Get(),
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"GFX Command Allocator" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command Allocator" : L"Command Allocator");
    }

    void D3D12CommandAllocator::Reset()
    {
        HRESULT hr{ S_OK };
        hr = command_allocator_->Reset();
        DXCall(hr);
        assert(SUCCEEDED(hr));
    }
}
