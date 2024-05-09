#include "D3D12Helpers.h"
#include "D3D12CommonHeaders.h"
#include "D3D12Core.h"
#include "D3D12Upload.h"

namespace RHI::D3D12::D3DX
{
    namespace {

    } // anonymous namespace

    void TransitionResource(ID3D12GraphicsCommandList6* cmd_list, ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after, D3D12_RESOURCE_BARRIER_FLAGS flags/* = D3D12_RESOURCE_BARRIER_FLAG_NONE*/, u32 subresource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/)
    {
        D3D12_RESOURCE_BARRIER barrier{};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = flags;
        barrier.Transition.pResource = resource;
        barrier.Transition.StateBefore = before;
        barrier.Transition.StateAfter = after;
        barrier.Transition.Subresource = subresource;

        cmd_list->ResourceBarrier(1, &barrier);
    }

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

    ID3D12Resource* CreateBuffer(
        const void* data, u32 buffer_size,
        bool is_cpu_accessible/* = false*/,
        D3D12_RESOURCE_STATES state/* = D3D12_RESOURCE_STATE_COMMON*/,
        D3D12_RESOURCE_FLAGS flags/* = D3D12_RESOURCE_FLAG_NONE*/,
        ID3D12Heap* heap/* = nullptr*/, u64 heap_offset/* = 0*/)
    {
        assert(buffer_size);

        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        desc.Alignment = 0;
        desc.Width = buffer_size;
        desc.Height = 1;
        desc.DepthOrArraySize = 1;
        desc.MipLevels = 1;
        desc.Format = DXGI_FORMAT_UNKNOWN;
        desc.SampleDesc = { 1,0 };
        desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        desc.Flags = is_cpu_accessible ? D3D12_RESOURCE_FLAG_NONE : flags;

        // バッファはアップロードにのみ使用されるか、常時バッファ/UAVとして使用される。
        assert(desc.Flags == D3D12_RESOURCE_FLAG_NONE ||
            desc.Flags == D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);

        ID3D12Resource* resource{ nullptr };
        const D3D12_RESOURCE_STATES resource_state
        { is_cpu_accessible ? D3D12_RESOURCE_STATE_GENERIC_READ : state };

        if (heap) {// バッファを既存のヒープに作成する場合
            DXCall(Core::GetDevice()->CreatePlacedResource(
                heap, heap_offset, &desc, resource_state,
                nullptr, IID_PPV_ARGS(&resource)));
        }
        else { // バッファを新しいヒープに作成する場合
            DXCall(Core::GetDevice()->CreateCommittedResource(
                is_cpu_accessible ? &heap_properties.UploadHeap : &heap_properties.DefaultHeap,
                D3D12_HEAP_FLAG_NONE, &desc, resource_state,
                nullptr, IID_PPV_ARGS(&resource)));
        }

        if (data) {
            // 初期データがあり、後で変更できるようにしたい場合は、is_cpu_accessibleをtrueに設定
            // GPUが使用するデータを一度だけアップロードしたい場合は、is_cpu_accessibleをfalseに設定

            if (is_cpu_accessible) {//初期データがあり、後で変更可能にする
                // NOTE: 範囲のBeginフィールドとEndフィールドに0がセットされ、CPUがデータを読み込んでいない（つまり書き込み専用）ことを示す。
                const D3D12_RANGE range{};
                void* cpu_address{ nullptr };
                DXCall(resource->Map(0, &range, reinterpret_cast<void**>(&cpu_address)));
                assert(cpu_address);
                memcpy(cpu_address, data, buffer_size);
                resource->Unmap(0, nullptr);
            }
            else {//GPUが使用するデータを一度だけアップロード
                UPLOAD::D3D12UploadContext context{ buffer_size };
                memcpy(context.CpuAddress(), data, buffer_size);
                context.CommandList()->CopyResource(resource, context.UploadBuffer());
                context.EndUpload();
            }
        }

        assert(resource);
        return resource;
    }


}

