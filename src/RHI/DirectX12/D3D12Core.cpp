#include "D3D12Core.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandList.h"
#include "D3D12CommandAllocator.h"
#include "D3D12Fence.h"

#include "D3D12Device.h"

using namespace RHI::D3D12::Parts;

namespace RHI::D3D12::Core {
    namespace {

        ComPtr<IDXGIFactory7> dxgi_factory{ nullptr };

        shared_ptr<D3D12Device> main_device;
        shared_ptr<D3D12CommandQueue> gfx_command_queue;
        shared_ptr<D3D12CommandList> gfx_command_list;
        shared_ptr<D3D12CommandAllocator> gfx_command_allocator;
        shared_ptr<D3D12Fence> gfx_fence;

        void BeginFrame()
        {
            //command_frame& frame{ command_frames[frame_index] };
            //frame.Wait(fence_event, fence.Get());
            gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());

            gfx_command_allocator->Reset();
            //DXCall(command_list_->Reset(frame.command_allocator_.Get(), nullptr));
            gfx_command_list->Reset(gfx_command_allocator->GetCommandAllocator());

        }

        void EndFrame()
        {
            //DXCall(command_list_->Close());
            gfx_command_list->Close();

            ID3D12CommandList* const cmd_lists[]{ gfx_command_list->GetCommandList() };
            gfx_command_queue->GetCommandQueue()->ExecuteCommandLists(_countof(cmd_lists), &cmd_lists[0]);

            //u64& fence_value{ fence_value };

            gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());

            /*
            ++fence_value;
            command_frame& frame{ command_frames[frame_index] };
            frame.fence_value_ = fence_value;
            command_queue_->Signal(fence.Get(), fence_value);

            frame_index = (frame_index + 1) % frame_buffer_count;
            */
        }

        bool FailedInit()
        {
            Shutdown();
            return false;
        }

        /*
        void CreateDevice()
        {
            //GPUベンダー定義
            enum class GPU_VENDER {
                NVIDIA,	//NVIDIA
                AMD,	//Intel
                INTEL,	//AMD

                NUM,	//Vender数
            };

            IDXGIAdapter* adapter_tmp = nullptr;
            IDXGIAdapter* adapter_vender[(int)GPU_VENDER::NUM] = { nullptr };   //各ベンダーのアダプター
            IDXGIAdapter* adapter_max_video_memory = nullptr;					//最大ビデオメモリのアダプタ
            IDXGIAdapter* use_adapter = nullptr;								//最終的に使用するアダプタ
            SIZE_T video_memory_size = 0;										//最大ビデオメモリのサイズ

            for (int i = 0; dxgi_factory->EnumAdapters(i, &adapter_tmp) != DXGI_ERROR_NOT_FOUND; i++) {
                //アダプターの情報を取得。
                DXGI_ADAPTER_DESC desc;
                adapter_tmp->GetDesc(&desc);

                //最大ビデオメモリのアダプターを保存。
                //ここで取得したアダプタはAMDやINTEL等のGPUがない場合に使用するアダプタ
                if (desc.DedicatedVideoMemory > video_memory_size) {
                    //すでに登録している場合は、解放してから登録。
                    if (adapter_max_video_memory != nullptr) {
                        adapter_max_video_memory->Release();
                    }
                    adapter_max_video_memory = adapter_tmp;
                    adapter_max_video_memory->AddRef();
                    video_memory_size = desc.DedicatedVideoMemory;
                }

                //ベンダー別にアダプターを保存。
                //すでに登録している場合は、解放してから登録
                if (wcsstr(desc.Description, L"NVIDIA") != nullptr) {//NVIDIA
                    if (adapter_vender[(int)GPU_VENDER::NVIDIA]) {
                        adapter_vender[(int)GPU_VENDER::NVIDIA]->Release();
                    }
                    adapter_vender[(int)GPU_VENDER::NVIDIA] = adapter_tmp;
                    adapter_vender[(int)GPU_VENDER::NVIDIA]->AddRef();
                }
                else if (wcsstr(desc.Description, L"AMD") != nullptr) {//AMD
                    if (adapter_vender[(int)GPU_VENDER::AMD]) {
                        adapter_vender[(int)GPU_VENDER::AMD]->Release();
                    }
                    adapter_vender[(int)GPU_VENDER::AMD] = adapter_tmp;
                    adapter_vender[(int)GPU_VENDER::AMD]->AddRef();
                }
                else if (wcsstr(desc.Description, L"Intel") != nullptr) {//Intel
                    if (adapter_vender[(int)GPU_VENDER::INTEL]) {
                        adapter_vender[(int)GPU_VENDER::INTEL]->Release();
                    }
                    adapter_vender[(int)GPU_VENDER::INTEL] = adapter_tmp;
                    adapter_vender[(int)GPU_VENDER::INTEL]->AddRef();
                }
                //アダプターを解放
                adapter_tmp->Release();
            }

            //使用するアダプタを決める(現在はNVIDIAが最優先)
            // NVIDIA > AMD > intel > other
            if (adapter_vender[(int)GPU_VENDER::NVIDIA] != nullptr)use_adapter = adapter_vender[(int)GPU_VENDER::NVIDIA];
            else if (adapter_vender[(int)GPU_VENDER::AMD] != nullptr) use_adapter = adapter_vender[(int)GPU_VENDER::AMD];
            else if (adapter_vender[(int)GPU_VENDER::INTEL] != nullptr)use_adapter = adapter_vender[(int)GPU_VENDER::INTEL];
            else use_adapter = adapter_max_video_memory;

            //使用している可能性のあるFEATURE_LEVELを列挙
            const D3D_FEATURE_LEVEL FEATURELEVELS[] = {
                D3D_FEATURE_LEVEL_12_1,
                D3D_FEATURE_LEVEL_12_0,
                D3D_FEATURE_LEVEL_11_1,
                D3D_FEATURE_LEVEL_11_0,
            };

            //D3D_FEATURE_LEVELのレベルの高い順にloop
            //HACK:https://forums.developer.nvidia.com/t/poco-notfoundexception-thrown-in-driver-version-531-18/245285
            //Poco::NotFoundExceptionの例外スローが発生中，原因はNVIDIAのドライバ
            for (auto feature_level : FEATURELEVELS) {
                if (SUCCEEDED(D3D12CreateDevice(
                    use_adapter,
                    feature_level,
                    IID_PPV_ARGS(&main_device)
                ))) {
                    break;
                }
            }

            //取得した全てのグラフィックカードを解放する
            //use_adapter->Release();
            for (auto& adapter : adapter_vender) {
                if (adapter) {//存在する場合解放
                    adapter->Release();
                }
            }

            //GPU以外でグラフィック能力のあるデバイス解放
            if (adapter_max_video_memory) {
                adapter_max_video_memory->Release();
            }

            //デバイスが取得できなかった場合


        }
        */
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

        // フェンスの作成
        gfx_fence = make_shared<D3D12Fence>(main_device->GetDevice());
        if (!gfx_fence) return FailedInit();
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

    // アプリケーションの終了時に呼び出される、リソースのクリーンアップを行う関数
    void Shutdown()
    {


        gfx_fence->WaitForGPU(gfx_command_queue->GetCommandQueue());
        gfx_fence.reset();

        gfx_command_list.reset();
        gfx_command_allocator.reset();
        gfx_command_queue.reset();


        // DXGIファクトリーインターフェースの解放
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
            DXCall(debug_device->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY | D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL));
        }
#endif // _DEBUG

        main_device.reset();
        dxgi_factory.Reset();
    }

    void Render()
    {
        BeginFrame();


        EndFrame();
    }
}
