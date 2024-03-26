#include "D3D12Fence.h"

namespace RHI::D3D12::Parts {
    D3D12Fence::D3D12Fence(ID3D12Device* const device)
    {
        HRESULT hr{ S_OK };
        DXCall(hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
        assert(SUCCEEDED(hr));
        NAME_D3D12_OBJECT(fence_, L"D3D12 Fence");

        event_ = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);
        assert(event_);
    }

    void D3D12Fence::WaitForGPU(ID3D12CommandQueue* const cmd_queue)
    {
        HRESULT hr{ S_OK };

        // フェンスのシグナル
        hr = cmd_queue->Signal(fence_.Get(), value_);
        assert(SUCCEEDED(hr));

        // フェンスの待機
        // 現在のフェンス値がまだ value より小さい場合、GPUはコマンドリストの実行を終了してないことがわかる
        if (fence_->GetCompletedValue() < value_) {
            fence_->SetEventOnCompletion(value_, event_);
            WaitForSingleObject(event_, INFINITE);
        }

        //フェンスの値を更新
        value_++;
    }
}
