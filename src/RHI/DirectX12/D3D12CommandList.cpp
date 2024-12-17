#include "D3D12CommandList.h"

namespace RHI::D3D12::Parts {

    D3D12CommandList::D3D12CommandList(ID3D12Device* const device, const D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator* const cmd_allocator)
    {
        HRESULT hr{ S_OK };
        hr = device->CreateCommandList(0, type, cmd_allocator, nullptr, IID_PPV_ARGS(&command_list_));
        DXCall(hr);
        assert(SUCCEEDED(hr));
        DXCall(command_list_->Close());
        NAME_D3D12_OBJECT(command_list_,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"GFX Command List" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command List" : L"Command List");

    }

    void D3D12CommandList::Reset(ID3D12CommandAllocator* const cmd_allocator)
    {
        HRESULT hr{ S_OK };
        hr = command_list_->Reset(cmd_allocator, nullptr);
        DXCall(hr);
        assert(SUCCEEDED(hr));
    }

    void D3D12CommandList::Close()
    {
        HRESULT hr{ S_OK };
        hr = command_list_->Close();
        DXCall(hr);
        assert(SUCCEEDED(hr));
    }

}
