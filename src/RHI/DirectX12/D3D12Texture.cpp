#include "D3D12Texture.h"
#include "D3D12Core.h"
#include "D3D12Helpers.h"
#include "D3D12DescriptorHeap.h"

namespace RHI::D3D12::Parts
{
    D3D12Texture::D3D12Texture(D3D12TextureInitInfo info)
    {
        auto* const device{ Core::GetDevice() };
        assert(device);

        D3D12_CLEAR_VALUE* const clear_value
        {
            (info.desc &&
            (info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
             info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            ? &info.clear_value : nullptr
        };

        if (info.resource) {// 既存のリソースを使う
            assert(!info.heap);
            resource_ = info.resource;
        }
        else if (info.heap && info.desc) {// 軽量なリソース(64KB以下)
            assert(!info.resource);
            DXCall(device->CreatePlacedResource(
                info.heap, info.allocation_info.Offset, info.desc,
                info.initial_state, clear_value, IID_PPV_ARGS(&resource_)));
        }
        else if (info.desc) {// 通常のリソース(64KB以上)
            assert(!info.heap && !info.resource);
            DXCall(device->CreateCommittedResource(
                &RHI::D3D12::D3DX::heap_properties.DefaultHeap, D3D12_HEAP_FLAG_NONE, info.desc,
                info.initial_state, clear_value, IID_PPV_ARGS(&resource_)));
        }

        assert(resource_);
        srv_ = Core::GetSRVHeap().Allocate();
        device->CreateShaderResourceView(resource_, info.srv_desc, srv_.cpu);
    }

    void D3D12Texture::Release()
    {
        if (srv_.IsValid())Core::GetSRVHeap().Free(srv_);
        if (resource_ != nullptr)Core::Detail::DeferredRelease(resource_);

    }

}