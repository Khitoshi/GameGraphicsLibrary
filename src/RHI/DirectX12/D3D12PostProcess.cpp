#include "D3D12PostProcess.h"
#include "D3D12Shaders.h"
#include "D3D12Gpass.h"
#include "D3D12Core.h"
#include "D3D12RenderTexture.h"

namespace RHI::D3D12::PostProcess
{
    namespace {

        struct FX_RootParamIndices
        {
            enum : u32 {
                ROOT_CONSTANTS,
                DESCRIPTOR_TABLE,

                count
            };
        };

        ID3D12RootSignature* fx_root_sig{ nullptr };
        ID3D12PipelineState* fx_pso{ nullptr };

        bool CreatFXPosAndRootSignature()
        {
            assert(!fx_root_sig && !fx_pso);
            // Create FX root signature
            D3DX::D3D12DescriptorRange range
            {
                D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND, 0, 0,
                D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE
            };

            using idx = FX_RootParamIndices;
            D3DX::D3D12RootParameter parameters[idx::count]{};
            parameters[idx::ROOT_CONSTANTS].AsConstants(1, D3D12_SHADER_VISIBILITY_PIXEL, 1);
            parameters[idx::DESCRIPTOR_TABLE].AsDescriptorTable(D3D12_SHADER_VISIBILITY_PIXEL, &range, 1);

            const D3DX::D3D12RootSignatureDesc root_signature{ &parameters[0], _countof(parameters) };
            fx_root_sig = root_signature.create();
            assert(fx_root_sig);
            NAME_D3D12_OBJECT(fx_root_sig, L"Post-process FX Root Signature");

            // Create FX PSO
            struct {
                D3DX::D3D12PipelineStateSubobject_RootSignature         root_signature{ fx_root_sig };
                D3DX::D3D12PipelineStateSubobject_VS                    vs{ Shaders::GetEngineShader(Shaders::EngineShader::FULLSCREENTRIANGLE_VS) };
                D3DX::D3D12PipelineStateSubobject_PS                    ps{ Shaders::GetEngineShader(Shaders::EngineShader::POSTPROCESS_PS) };
                D3DX::D3D12PipelineStateSubobject_PrimitiveTopology     primitive_topology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
                D3DX::D3D12PipelineStateSubobject_RenderTargetFormats   render_target_formats;
                D3DX::D3D12PipelineStateSubobject_Rasterizer            rasterizer{ D3DX::rasterizer_state.NoCull };
            } stream;

            D3D12_RT_FORMAT_ARRAY rtf_array{};
            rtf_array.NumRenderTargets = 1;
            rtf_array.RTFormats[0] = Parts::default_back_buffer_format;

            stream.render_target_formats = rtf_array;

            fx_pso = D3DX::CreatePipelineState(&stream, sizeof(stream));
            NAME_D3D12_OBJECT(fx_pso, L"Post-process FX Pipeline State Object");

            return fx_root_sig && fx_pso;
        }
    }

    bool Initialize()
    {
        return CreatFXPosAndRootSignature();
    }

    void Shutdown()
    {
        if (fx_root_sig != nullptr) {
            fx_root_sig->Release();
            fx_root_sig = nullptr;
        }
        if (fx_pso != nullptr) {
            fx_pso->Release();
            fx_pso = nullptr;
        }
    }

    void PostProcess(ID3D12GraphicsCommandList6* cmd_list, D3D12_CPU_DESCRIPTOR_HANDLE target_rtv)
    {
        cmd_list->SetGraphicsRootSignature(fx_root_sig);
        cmd_list->SetPipelineState(fx_pso);
        auto& main_buffer = RHI::D3D12::GPASS::MainBuffer();
        using idx = FX_RootParamIndices;
        cmd_list->SetGraphicsRoot32BitConstant(idx::ROOT_CONSTANTS, main_buffer.SRV().index, 0);
        cmd_list->SetGraphicsRootDescriptorTable(idx::DESCRIPTOR_TABLE, Core::GetSRVHeap().GetGPUStart());
        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        // NOTE: 各ピクセルはgpassメインバッファからのピクセルで上書きされるので、
        // レンダーターゲットをクリアする必要はない。 深度バッファも必要ない。
        cmd_list->OMSetRenderTargets(1, &target_rtv, 1, nullptr);
        cmd_list->DrawInstanced(3, 1, 0, 0);
    }

}

