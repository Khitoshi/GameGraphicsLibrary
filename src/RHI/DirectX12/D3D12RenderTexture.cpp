#include "D3D12RenderTexture.h"
#include "D3D12Core.h"

namespace RHI::D3D12::Parts
{
    D3D12RenderTexture::D3D12RenderTexture(D3D12TextureInitInfo info) :
        texture_(info)
    {
        assert(info.desc);

        D3D12_RENDER_TARGET_VIEW_DESC desc{};
        desc.Format = info.desc->Format;
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;

        // RTV�̍쐬
        auto* const device{ Core::GetDevice() };
        D3D12DescriptorHeap& rtv_heap{ Core::GetRTVHeap() };
        mip_count_ = Resource()->GetDesc().MipLevels;
        assert(mip_count_ && mip_count_ <= D3D12Texture::MAXMIPS);
        for (u32 i{ 0 }; i < mip_count_; ++i) {
            rtv_[i] = rtv_heap.Allocate();
            device->CreateRenderTargetView(Resource(), &desc, rtv_[i].cpu);
            ++desc.Texture2D.MipSlice;
        }
    }

    void D3D12RenderTexture::Release()
    {
        for (u32 i{ 0 }; i < mip_count_; ++i) {
            Core::GetRTVHeap().Free(rtv_[i]);
        }

        texture_.Release();
        mip_count_ = 0;
    }
}
