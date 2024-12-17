#include "D3D12DepthBuffer.h"
#include "D3D12Core.h"

namespace RHI::D3D12::Parts {

    D3D12DepthBuffer::D3D12DepthBuffer(D3D12TextureInitInfo info)
    {
        assert(info.desc);
        const DXGI_FORMAT dsv_format{ info.desc->Format };

        {// create texture resouce
            D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
            if (info.desc->Format == DXGI_FORMAT_D32_FLOAT) {
                info.desc->Format = DXGI_FORMAT_R32_TYPELESS;
                srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
            }

            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            srv_desc.Texture2D.MipLevels = 1;
            srv_desc.Texture2D.MostDetailedMip = 0;
            srv_desc.Texture2D.PlaneSlice = 0;
            srv_desc.Texture2D.ResourceMinLODClamp = 0.f;

            assert(!info.srv_desc && !info.resource);
            info.srv_desc = &srv_desc;
            texture_ = D3D12Texture(info);
        }

        {// create depth stencil view(dsv)
            D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
            dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
            dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
            dsv_desc.Format = dsv_format;
            dsv_desc.Texture2D.MipSlice = 0;

            dsv_ = Core::GetDSVHeap().Allocate();

            auto* const device{ Core::GetDevice() };
            assert(device);
            device->CreateDepthStencilView(resource(), &dsv_desc, dsv_.cpu);
        }
    }

    void D3D12DepthBuffer::Release()
    {
        if (dsv_.IsValid()) {
            Core::GetDSVHeap().Free(dsv_);
        }
        texture_.Release();
    }

}