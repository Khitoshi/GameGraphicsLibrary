#include "D3D12Core.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandList.h"
#include "D3D12CommandAllocator.h"
#include "D3D12Fence.h"
#include "D3D12DescriptorHeap.h"
#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "D3D12Shaders.h"

#include "../Platform/Window.h"
#include "../../Renderer/Renderer.h"

using namespace RHI::D3D12::Parts;

namespace RHI::D3D12::Core {
    namespace {
        constexpr DXGI_FORMAT render_target_format{ DXGI_FORMAT_R8G8B8A8_UNORM_SRGB };

        ComPtr<IDXGIFactory7> dxgi_factory{ nullptr };

        shared_ptr<D3D12Device> main_device;
        shared_ptr<D3D12CommandQueue> gfx_command_queue;
        shared_ptr<D3D12CommandList> gfx_command_list;
        shared_ptr<D3D12CommandAllocator> gfx_command_allocator;
        shared_ptr<D3D12Fence> gfx_fence;
        shared_ptr<D3D12SwapChain> gfx_swap_chain;

        shared_ptr<DescriptorHeapManager> descriptor_heap_manager;

        Utility::vector<IUnknown*>  deferred_releases[frame_buffer_count]{};
        u32                         deferred_releases_flag[frame_buffer_count]{};
        std::mutex                  deferred_releases_mutex{};

        void BeginFrame()
        {
            D3D12CommandAllocator* allocator{ gfx_command_allocator.get() };
            allocator->Reset();

            gfx_command_list->Reset(allocator->GetCommandAllocator());
        }

        void EndFrame()
        {
            //DXCall(command_list_->Close());
            gfx_command_list->Close();

            ID3D12CommandList* const cmd_lists[]{ gfx_command_list->GetCommandList() };
            gfx_command_queue->GetCommandQueue()->ExecuteCommandLists(_countof(cmd_lists), &cmd_lists[0]);

            gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());

            gfx_fence->SetIndex((gfx_fence->GetIndex() + 1) % frame_buffer_count);
        }

        bool FailedInit()
        {
            Shutdown();
            return false;
        }

        void __declspec(noinline) process_deferred_releases(u32 frame_idx)
        {
            std::lock_guard lock{ deferred_releases_mutex };

            // NOTE: we clear this flag in the beginning. If we'd clear it at the end
            //       then it might overwrite some other thread that was trying to set it.
            //       It's fine if overwriting happens before processing the items.
            deferred_releases_flag[frame_idx] = 0;
            descriptor_heap_manager->GetRTVDescriptorHeap().ProcessDeferredFree(frame_idx);
            descriptor_heap_manager->GetDSVDescriptorHeap().ProcessDeferredFree(frame_idx);
            descriptor_heap_manager->GetSRVDescriptorHeap().ProcessDeferredFree(frame_idx);
            descriptor_heap_manager->GetUAVDescriptorHeap().ProcessDeferredFree(frame_idx);

            Utility::vector<IUnknown*>& resources{ deferred_releases[frame_idx] };
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

    }

    bool Initialize()
    {
        if (main_device) Shutdown();

        u32 dxgi_factory_flags{ 0 };
#ifdef _DEBUG
        {
            ComPtr<ID3D12Debug> debug_controller;
            if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)))) {
                debug_controller->EnableDebugLayer();

                //デバッグの場合デバッグフラグを立てる
                dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;

                ComPtr<ID3D12Debug3> debug3;
                debug_controller.As(&debug3);
                if (debug3) {
                    debug3->SetEnableGPUBasedValidation(true);
                }

                ComPtr<ID3D12DeviceRemovedExtendedDataSettings1> d3dDredSettings1;
                if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&d3dDredSettings1)))) {
                    // 自動パンくずリストとページ障害報告をオンにする
                    d3dDredSettings1->SetAutoBreadcrumbsEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    d3dDredSettings1->SetBreadcrumbContextEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                    d3dDredSettings1->SetPageFaultEnablement(D3D12_DRED_ENABLEMENT_FORCED_ON);
                }
            }
        }
#endif // _DEBUG

        //DXGIファクトリの作成
        HRESULT hr{ S_OK };

        DXCall(hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&dxgi_factory)));
        if (FAILED(hr)) return FailedInit();

        //デバイスの作成
        main_device = std::make_shared<D3D12Device>(dxgi_factory.Get());
        if (main_device == nullptr) return FailedInit();

        // コマンドキューの作成
        gfx_command_queue = make_shared<D3D12CommandQueue>(main_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (!gfx_command_queue) return FailedInit();

        // コマンドアロケータの作成
        gfx_command_allocator = make_shared<D3D12CommandAllocator>(main_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        if (!gfx_command_allocator) return FailedInit();

        // コマンドリストの作成
        gfx_command_list = make_shared<D3D12CommandList>(main_device->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT, gfx_command_allocator->GetCommandAllocator());
        if (!gfx_command_list) return FailedInit();

        if (!Shaders::Initialize()) return FailedInit();

        // フェンスの作成
        gfx_fence = make_shared<D3D12Fence>(main_device->GetDevice());
        if (!gfx_fence) return FailedInit();

        descriptor_heap_manager = make_shared<DescriptorHeapManager>();


#ifdef _DEBUG
        {
            ComPtr<ID3D12InfoQueue> info_queue;
            DXCall(main_device->GetDevice()->QueryInterface(IID_PPV_ARGS(&info_queue)));

            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
            info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
        }
#endif // _DEBUG

        return true;
    }

    namespace Detail {
        void DeferredRelease(IUnknown* resource)
        {
            const u32 frame_idx{ GetCurrentFrameIndex() };
            std::lock_guard lock{ deferred_releases_mutex };
            deferred_releases[frame_idx].push_back(resource);
            SetDeferredReleasesFlag();
        }
    }

    // アプリケーションの終了時に呼び出される、リソースのクリーンアップを行う関数
    void Shutdown()
    {

        gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());

        gfx_command_list.reset();
        gfx_command_allocator.reset();
        gfx_command_queue.reset();

        for (u32 i{ 0 }; i < frame_buffer_count; ++i)
        {
            descriptor_heap_manager->ProcessDeferredReleases(i);
        }
        descriptor_heap_manager->HeapDeferredAllRelease();
        descriptor_heap_manager->ProcessDeferredReleases(0);
        descriptor_heap_manager.reset();

        gfx_fence.reset();

        // DXGIファクトリーインターフェースの解放
        dxgi_factory.Reset();
#ifdef _DEBUG
        {
            // ID3D12InfoQueueインターフェースへのクエリを実行し、取得
            {
                ComPtr<ID3D12InfoQueue> info_queue;
                DXCall(main_device->GetDevice()->QueryInterface(IID_PPV_ARGS(&info_queue)));
                // エラー、警告、破損の重大度に基づく実行中断の設定を無効にする
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
                info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
            }

            // ID3D12DebugDevice2インターフェースへのクエリを実行し、取得
            ComPtr<ID3D12DebugDevice2> debug_device;
            DXCall(main_device->GetDevice()->QueryInterface(IID_PPV_ARGS(&debug_device)));
            // メインデバイスの解放前に、未解放のリソースに関するデバッグ情報をレポート
            main_device.reset();
            DXCall(debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
            debug_device.Reset();
        }
#endif // _DEBUG

        if (main_device)main_device.reset();

    }

#pragma region Sureface
    Renderer::Surface CreateSurface(::Platform::Window window)
    {
        //gfx_swap_chaines.emplace_back(window);
        //Renderer::SurfaceID id{ (u32)gfx_swap_chaines.size() - 1 };
        Renderer::SurfaceID id{ 0 };
        //gfx_swap_chaines[id].CreateSwapChain(dxgi_factory.Get(), gfx_command_queue->GetCommandQueue(), render_target_format);
        //return Renderer::Surface{ id };
        gfx_swap_chain = make_shared<D3D12SwapChain>(window);
        gfx_swap_chain->CreateSwapChain(dxgi_factory.Get(), gfx_command_queue->GetCommandQueue(), render_target_format);
        return Renderer::Surface{ id };
    }

    void RemoveSurface(Renderer::SurfaceID id)
    {
        //TODO:複数window対応にする場合はidを使う
        //gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());
        //gfx_swap_chaines[id].~D3D12SwapChain();
        gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());
        //gfx_swap_chain.~D3D12SwapChain();
        gfx_swap_chain.reset();
    }

    void ResizeSurface(Renderer::SurfaceID id, u32, u32)
    {
        //gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());
        //gfx_swap_chaines[id].Resize();

        gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());
        gfx_swap_chain->Resize();
    }

    u32 SurfaceWidth(Renderer::SurfaceID id)
    {
        //return gfx_swap_chaines[id].Width();
        return gfx_swap_chain->Width();
    }

    u32 SurfaceHeight(Renderer::SurfaceID id)
    {
        //return gfx_swap_chaines[id].Height();
        return gfx_swap_chain->Height();
    }

    void RenderSurface(Renderer::SurfaceID id)
    {
        BeginFrame();

        ID3D12GraphicsCommandList6* cmd_list{ gfx_command_list->GetCommandList() };

        const u32 frame_idx{ GetCurrentFrameIndex() };
        if (descriptor_heap_manager->GetDeferredReleasesFlag(frame_idx) != 0) {
            descriptor_heap_manager->ProcessDeferredReleases(frame_idx);
        }

        //const D3D12SwapChain& surface{ gfx_swap_chain };
        //surface.Present();
        gfx_swap_chain->Present();

        EndFrame();
    }
#pragma endregion//Suerface

#pragma region Getter
    ID3D12Device8* const GetDevice()
    {
        assert(main_device);
        return main_device->GetDevice();
    }

    u32 GetCurrentFrameIndex()
    {
        assert(gfx_fence);
        return gfx_fence->GetIndex();
    }
    Parts::DescriptorHeapManager* GetDescriptorHeapManager()
    {
        assert(descriptor_heap_manager.get());
        return descriptor_heap_manager.get();
    }
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetSRVHeap()
    {
        assert(descriptor_heap_manager.get());
        return descriptor_heap_manager->GetSRVDescriptorHeap();
    }
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetRTVHeap()
    {
        assert(descriptor_heap_manager.get());
        return descriptor_heap_manager->GetRTVDescriptorHeap();
    }
    RHI::D3D12::Parts::D3D12DescriptorHeap& GetDSVHeap()
    {
        assert(descriptor_heap_manager.get());
        return descriptor_heap_manager->GetDSVDescriptorHeap();
    }
#pragma endregion//Getter

#pragma region Setter
    void SetDeferredReleasesFlag()
    {
        deferred_releases_flag[GetCurrentFrameIndex()] = 1;
    }
#pragma endregion//Setter
}
