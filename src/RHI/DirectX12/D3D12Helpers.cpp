#include "D3D12Helpers.h"
#include "D3D12CommonHeaders.h"
#include "D3D12Core.h"

namespace RHI::D3D12::D3DX
{
    namespace {

    } // anonymous namespace

    ID3D12RootSignature* CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& desc)
    {
        D3D12_VERSIONED_ROOT_SIGNATURE_DESC versioned_desc{};
        versioned_desc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
        versioned_desc.Desc_1_1 = desc;

        ComPtr<ID3DBlob> signature_blob{ nullptr };
        ComPtr<ID3DBlob> error_blob{ nullptr };
        HRESULT hr{ S_OK };
        if (FAILED(hr = D3D12SerializeVersionedRootSignature(&versioned_desc, &signature_blob, &error_blob)))
        {
            DEBUG_OP(const char* error_msg{ error_blob ? (const char*)error_blob->GetBufferPointer() : "" });
            DEBUG_OP(OutputDebugStringA(error_msg));
            return nullptr;
        }

        ID3D12RootSignature* signature{ nullptr };
        DXCall(hr = Core::GetDevice()->CreateRootSignature(0, signature_blob->GetBufferPointer(), signature_blob->GetBufferSize(), IID_PPV_ARGS(&signature)));

        if (FAILED(hr))
        {
            //Core::release(signature);
            signature->Release();
            signature = nullptr;
        }

        return signature;
    }

    ID3D12PipelineState* CreatePipelineState(D3D12_PIPELINE_STATE_STREAM_DESC desc)
    {
        assert(desc.pPipelineStateSubobjectStream && desc.SizeInBytes);
        ID3D12PipelineState* pso{ nullptr };
        DXCall(Core::GetDevice()->CreatePipelineState(&desc, IID_PPV_ARGS(&pso)));
        assert(pso);
        return pso;
    }


    ID3D12PipelineState* CreatePipelineState(void* stream, u64 stream_size)
    {
        assert(stream && stream_size);
        D3D12_PIPELINE_STATE_STREAM_DESC desc{};
        desc.SizeInBytes = stream_size;
        desc.pPipelineStateSubobjectStream = stream;
        return CreatePipelineState(desc);
    }

}

