#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12Texture.h"

namespace RHI::D3D12::Parts {

    class D3D12RenderTexture
    {
    public:
        D3D12RenderTexture() = default;
        explicit D3D12RenderTexture(D3D12TextureInitInfo info);
        DISABLE_COPY(D3D12RenderTexture);
        constexpr D3D12RenderTexture(D3D12RenderTexture&& o) :
            texture_{ std::move(o.texture_) },
            mip_count_{ o.mip_count_ }
        {
            for (u32 i{ 0 }; i < mip_count_; ++i) rtv_[i] = o.rtv_[i];
            o.Reset();
        }

        constexpr D3D12RenderTexture& operator=(D3D12RenderTexture&& o)
        {
            assert(this != &o);
            if (this != &o)
            {
                Release();
                Move(o);
            }
            return *this;
        }

        ~D3D12RenderTexture() { Release(); }

        void Release();

#pragma region Getter
        [[nodiscard]] constexpr u32 MipCount() const { return mip_count_; }
        [[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE RTV(u32 mip_index) const { assert(mip_index < mip_count_); return rtv_[mip_index].cpu; }
        [[nodiscard]] constexpr DescriptorHandle SRV() const { return texture_.srv(); }
        [[nodiscard]] constexpr ID3D12Resource* const Resource() const { return texture_.resource(); }
#pragma endregion //Getter


    private:
        constexpr void Move(D3D12RenderTexture& o)
        {
            texture_ = std::move(o.texture_);
            mip_count_ = o.mip_count_;
            for (u32 i{ 0 }; i < mip_count_; ++i) rtv_[i] = o.rtv_[i];
            o.Reset();
        }

        constexpr void Reset()
        {
            for (u32 i{ 0 }; i < mip_count_; ++i) rtv_[i] = {};
            mip_count_ = 0;
        }

        D3D12Texture       texture_{};
        DescriptorHandle   rtv_[D3D12Texture::MAXMIPS]{};
        u32                mip_count_{ 0 };
    };

}