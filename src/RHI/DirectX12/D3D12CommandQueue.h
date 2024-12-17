#pragma once
#include "D3D12CommonHeaders.h"
#include "../RHICommandQueue.h"

namespace RHI::D3D12::Parts {
    class D3D12CommandQueue :public RHI::RHICommandQueue
    {
    public:
        D3D12CommandQueue() = default;
        explicit D3D12CommandQueue(ID3D12Device8* const device, D3D12_COMMAND_LIST_TYPE type);
        ~D3D12CommandQueue() override = default;

    public:
#pragma region Getter
        ID3D12CommandQueue* const GetCommandQueue() const { return command_queue_.Get(); }
#pragma endregion //Getter

    private:
        Microsoft::WRL::ComPtr<ID3D12CommandQueue> command_queue_{ nullptr };
    };

}