#pragma once

#include "D3D12Core.h"
#include "D3D12Helpers.h"

namespace RHI::D3D12::Parts {
    ID3D12RootSignature* root_signature_;
    D3D12_SHADER_BYTECODE vs_{};

    template<D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type, typename T>
    class alignas(void*) D3D12PipelineStateSubobject
    {
    public:
        D3D12PipelineStateSubobject() = default;
        constexpr explicit D3D12PipelineStateSubobject(T subobject) : type_{ type }, subobject_{ subobject } {}
        D3D12PipelineStateSubobject& operator=(const T& subobject) { subobject_ = subobject; return *this; }
    private:
        const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type_{ type };
        T subobject_{};
    };

    using D3D12PipelineStateSubobject_RootSignature = D3D12PipelineStateSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE, ID3D12RootSignature*>;
    using D3D12PipelineStateSubobject_VS = D3D12PipelineStateSubobject< D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS, D3D12_SHADER_BYTECODE>;

    void CreatePipelineStateObject()
    {
        struct {
            struct alignas(void*) {
                const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type{ D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE };
                ID3D12RootSignature* root_signature{ nullptr };
            } root_sig;
            struct alignas(void*) {
                const D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type{ D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS };
                D3D12_SHADER_BYTECODE vs_code{};
            } vs;
        } stream;

        stream.root_sig.root_signature = root_signature_;
        stream.vs.vs_code = vs_;

        D3D12_PIPELINE_STATE_STREAM_DESC desc{};
        desc.pPipelineStateSubobjectStream = &stream;
        desc.SizeInBytes = sizeof(stream);

        ID3D12PipelineState* pso{ nullptr };
        Core::GetDevice()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso));

        // レンダリング時にpsoを使用する

        // レンダラーがシャットダウンしたとき
        if (pso) {
            pso->Release();
            pso = nullptr;
        }
    }

    void CreatePipelineStateObject2()
    {
        struct {
            D3DX::D3D12PipelineStateSubobject_RootSignature root_sig{ root_signature_ };
            D3DX::D3D12PipelineStateSubobject_VS vs{ vs_ };
        } stream;

        auto pso = D3DX::CreatePipelineState(&stream, sizeof(stream));

        // レンダリング時にpsoを使用する

        // レンダラーがシャットダウンしたとき
        if (pso) {
            pso->Release();
            pso = nullptr;
        }
    }
}