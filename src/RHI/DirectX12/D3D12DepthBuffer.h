#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Texture.h"

namespace RHI::D3D12::Parts {

    class D3D12DepthBuffer
    {
    public:
        D3D12DepthBuffer() = default;
        explicit D3D12DepthBuffer(D3D12TextureInitInfo info);
        DISABLE_COPY(D3D12DepthBuffer);
        constexpr D3D12DepthBuffer(D3D12DepthBuffer&& o)
            : texture_{ std::move(o.texture_) }, dsv_{ o.dsv_ }
        {
            o.dsv_ = {};
        }

        constexpr D3D12DepthBuffer& operator=(D3D12DepthBuffer&& o)
        {
            assert(this != &o);
            if (this != &o)
            {
                texture_ = std::move(o.texture_);
                dsv_ = o.dsv_;
                o.dsv_ = {};
            }
            return *this;
        }

        ~D3D12DepthBuffer() { Release(); }

        void Release();
        constexpr D3D12_CPU_DESCRIPTOR_HANDLE dsv() const { return dsv_.cpu; }
        constexpr DescriptorHandle srv() const { return texture_.srv(); }
        constexpr ID3D12Resource* const resource() const { return texture_.resource(); }

    private:
        D3D12Texture      texture_{};
        DescriptorHandle   dsv_{};
    };
}
