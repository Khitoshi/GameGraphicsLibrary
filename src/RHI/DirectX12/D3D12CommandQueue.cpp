#include "D3D12CommandQueue.h"
#include "D3D12Core.h"
using namespace Microsoft::WRL;

namespace RHI::D3D12::Parts {
    D3D12CommandQueue::D3D12CommandQueue(ID3D12Device8* const device, D3D12_COMMAND_LIST_TYPE type)
    {
        D3D12_COMMAND_QUEUE_DESC desc{};
        desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        desc.NodeMask = 0;
        desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
        desc.Type = type;

        HRESULT hr{ S_OK };
        hr = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&command_queue_));
        DXCall(hr);
        assert(SUCCEEDED(hr));
        NAME_D3D12_OBJECT(command_queue_,
            type == D3D12_COMMAND_LIST_TYPE_DIRECT ?
            L"GFX Command Queue" :
            type == D3D12_COMMAND_LIST_TYPE_COMPUTE ?
            L"Compute Command Queue" : L"Command Queue");
    }
}