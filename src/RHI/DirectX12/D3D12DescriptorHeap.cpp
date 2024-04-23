#include "D3D12DescriptorHeap.h"
#include "D3D12Core.h"

namespace RHI::D3D12::Parts
{
    D3D12DescriptorHeap::D3D12DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE type, const u32 capacity, bool is_shader_visible) :
        type_{ type }
    {
        std::lock_guard lock{ mutex_ };
        assert(capacity && capacity < D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2);
        assert(!(type_ == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE));

        if (type_ == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || type_ == D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {
            is_shader_visible = false;
        }

        ID3D12Device* const device{ RHI::D3D12::Core::GetDevice() };
        assert(device);

        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.Flags = is_shader_visible
            ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
            : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NumDescriptors = capacity;
        desc.Type = type_;
        desc.NodeMask = 0;

        HRESULT hr{ S_OK };
        device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptor_heap_));
        DXCall(hr);
        assert(SUCCEEDED(hr));

        free_handles_ = std::move(std::make_unique<u32[]>(capacity));
        capacity_ = capacity;
        size_ = 0;

        for (u32 i{ 0 }; i < capacity; ++i) free_handles_[i] = i;
        DEBUG_OP(for (u32 i{ 0 }; i < frame_buffer_count; ++i) assert(deferred_free_indices_[i].empty()));

        descriptor_size_ = device->GetDescriptorHandleIncrementSize(type_);
        cpu_start_ = descriptor_heap_->GetCPUDescriptorHandleForHeapStart();
        gpu_start_ = is_shader_visible ? descriptor_heap_->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
    }

    void D3D12DescriptorHeap::ProcessDeferredFree(u32 frame_idx)
    {
        std::lock_guard lock{ mutex_ };
        if (deferred_free_indices_[frame_idx].empty()) return;

        for (auto& index : deferred_free_indices_[frame_idx])
        {
            free_handles_[size_++] = index;
        }
        deferred_free_indices_[frame_idx].clear();
    }

    DescriptorHandle D3D12DescriptorHeap::Allocate()
    {
        std::lock_guard lock{ mutex_ };
        assert(descriptor_heap_);
        assert(size_ < capacity_);

        const u32 index{ free_handles_[size_] };
        const u32 offset{ index * descriptor_size_ };
        size_++;

        DescriptorHandle handle;
        handle.cpu.ptr = cpu_start_.ptr + offset;
        if (IsShaderVisible()) {
            handle.gpu.ptr = gpu_start_.ptr + offset;
        }

        DEBUG_OP(handle.container = this);
        DEBUG_OP(handle.index = index);
        return handle;
    }

    void D3D12DescriptorHeap::Free(DescriptorHandle& handle)
    {
        if (!handle.IsValid()) return;
        std::lock_guard lock{ mutex_ };
        assert(descriptor_heap_ && size_);
        assert(handle.container == this);
        assert(handle.cpu.ptr >= cpu_start_.ptr);
        assert((handle.cpu.ptr - cpu_start_.ptr) % descriptor_size_ == 0);
        assert(handle.index < capacity_);
        const u32 index{ (u32)(handle.cpu.ptr - cpu_start_.ptr) / descriptor_size_ };
        assert(handle.index == index);

        const u32 frame_idx{ Core::GetCurrentFrameIndex() };
        deferred_free_indices_[frame_idx].push_back(index);
        Core::GetDescriptorHeapManager()->SetDeferredReleasesFlag();
        handle = {};
    }

    void DescriptorHeapManager::ProcessDeferredReleases(u32 frame_idx)
    {
        std::lock_guard lock{ mutex_ };

        // このフラグは最初にクリアする。
        // もし最後にこのフラグをクリアしてしまうと、このフラグをセットしようとしていた他のスレッドを上書きしてしまうかもしれない。 
        // 上書きはアイテムを処理する前に行われれば問題ない。
        deferred_releases_flag_[frame_idx] = 0;

        rtv_desc_heap_.ProcessDeferredFree(frame_idx);
        dsv_desc_heap_.ProcessDeferredFree(frame_idx);
        srv_desc_heap_.ProcessDeferredFree(frame_idx);
        uav_desc_heap_.ProcessDeferredFree(frame_idx);

        Utility::vector<IUnknown*>& resources{ deferred_releases_[frame_idx] };
        if (!resources.empty())
        {
            for (auto& resource : resources) {
                //release(resource);
                resource->Release();
                resource = nullptr;
            }
            resources.clear();
        }
    }

    void DescriptorHeapManager::SetDeferredReleasesFlag()
    {
        deferred_releases_flag_[Core::GetCurrentFrameIndex()] = 1;
    }

    void DescriptorHeapManager::DeferredRelease(IUnknown* resource)
    {
        const u32 frame_idx{ Core::GetCurrentFrameIndex() };
        std::lock_guard lock{ mutex_ };
        deferred_releases_[frame_idx].push_back(resource);
        SetDeferredReleasesFlag();
    }

}

