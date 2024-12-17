#pragma once
#include "D3D12CommonHeaders.h"

namespace RHI::D3D12::Parts {

    class D3D12DescriptorHeap;

    struct DescriptorHandle {
        D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
        u32                         index{ u32_invalid_id };

#pragma region Getter
        [[nodiscard]] constexpr bool IsValid() const { return cpu.ptr != 0; }
        [[nodiscard]] constexpr bool IsShaderVisible() const { return gpu.ptr != 0; }
#pragma endregion //Getter
#ifdef _DEBUG
    private:
        friend class D3D12DescriptorHeap;
        D3D12DescriptorHeap* container{ nullptr };
#endif
    };

    class D3D12DescriptorHeap
    {
    public:
        D3D12DescriptorHeap() = default;
        explicit D3D12DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE type, const u32 capacity, bool is_shader_visible);
        ~D3D12DescriptorHeap() = default;

        void ProcessDeferredFree(u32 frame_idx);

        DescriptorHandle Allocate();

        void Free(DescriptorHandle& handle);

#pragma region Getter
        [[nodiscard]] ID3D12DescriptorHeap* const GetDescriptorHeap() const { return descriptor_heap_.Get(); }
        [[nodiscard]] constexpr D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return type_; }
        [[nodiscard]] constexpr bool IsShaderVisible() const { return gpu_start_.ptr != 0; }
        [[nodiscard]] const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUStart() const { return gpu_start_; }
#pragma endregion
    private:
        ComPtr<ID3D12DescriptorHeap> descriptor_heap_{ nullptr };
        const D3D12_DESCRIPTOR_HEAP_TYPE type_{};
        std::mutex mutex_{};

        u32 capacity_{ 0 };
        u32 size_{ 0 };

        D3D12_CPU_DESCRIPTOR_HANDLE cpu_start_{};
        D3D12_GPU_DESCRIPTOR_HANDLE gpu_start_{};

        u32 descriptor_size_{};

        std::unique_ptr<u32[]> free_handles_{};
        std::vector<u32> deferred_free_indices_[frame_buffer_count]{};
    };


    class DescriptorHeapManager
    {
    public:
        DescriptorHeapManager()
        {
            NAME_D3D12_OBJECT(rtv_desc_heap_.GetDescriptorHeap(), L"RTV Descriptor Heap");
            NAME_D3D12_OBJECT(dsv_desc_heap_.GetDescriptorHeap(), L"DSV Descriptor Heap");
            NAME_D3D12_OBJECT(srv_desc_heap_.GetDescriptorHeap(), L"SRV Descriptor Heap");
            NAME_D3D12_OBJECT(uav_desc_heap_.GetDescriptorHeap(), L"UAV Descriptor Heap");
        }
        ~DescriptorHeapManager() = default;

        void __declspec(noinline) ProcessDeferredReleases(u32 frame_idx);

        void SetDeferredReleasesFlag();

        void DeferredRelease(IUnknown* resource);

        void HeapDeferredAllRelease()
        {
            DeferredRelease(rtv_desc_heap_.GetDescriptorHeap());
            DeferredRelease(dsv_desc_heap_.GetDescriptorHeap());
            DeferredRelease(srv_desc_heap_.GetDescriptorHeap());
            DeferredRelease(uav_desc_heap_.GetDescriptorHeap());

            rtv_desc_heap_.~D3D12DescriptorHeap();
            dsv_desc_heap_.~D3D12DescriptorHeap();
            srv_desc_heap_.~D3D12DescriptorHeap();
            uav_desc_heap_.~D3D12DescriptorHeap();
        }

#pragma region Getter
        D3D12DescriptorHeap& GetRTVDescriptorHeap() { return rtv_desc_heap_; }
        D3D12DescriptorHeap& GetDSVDescriptorHeap() { return dsv_desc_heap_; }
        D3D12DescriptorHeap& GetSRVDescriptorHeap() { return srv_desc_heap_; }
        D3D12DescriptorHeap& GetUAVDescriptorHeap() { return uav_desc_heap_; }
        u32 GetDeferredReleasesFlag(u32 frame_idx) const { return deferred_releases_flag_[frame_idx]; }
#pragma endregion

    private:
        D3D12DescriptorHeap         rtv_desc_heap_{ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ,512,false };
        D3D12DescriptorHeap         dsv_desc_heap_{ D3D12_DESCRIPTOR_HEAP_TYPE_DSV ,512,false };
        D3D12DescriptorHeap         srv_desc_heap_{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,4096,true };
        D3D12DescriptorHeap         uav_desc_heap_{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,512,false };

        Utility::vector<IUnknown*>  deferred_releases_[frame_buffer_count]{};
        u32                         deferred_releases_flag_[frame_buffer_count]{};
        std::mutex                  mutex_{};
    };
}
