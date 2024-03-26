#include "D3D12Device.h"

namespace RHI::D3D12::Parts
{
    D3D12Device::D3D12Device(IDXGIFactory7* const dxgi_factory)
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
                IID_PPV_ARGS(&this->device_)
            ))) {
                NAME_D3D12_OBJECT(this->device_.Get(), L"Main D3D12 Device");
                break;
            }
        }

        //取得した全てのグラフィックカードを解放する
        for (auto& adapter : adapter_vender) {
            if (adapter) {//存在する場合解放
                adapter->Release();
            }
        }

        //GPU以外でグラフィック能力のあるデバイス解放
        if (adapter_max_video_memory) {
            adapter_max_video_memory->Release();
        }
    }
}
