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
        constexpr D3D12DepthBuffer(D3D12DepthBuffer&& o) :
            texture_{ std::move(o.texture_) },
            dsv_{ o.dsv_ }
        {
            o.dsv_ = {};
        }
        ~D3D12DepthBuffer() {
            if (texture_.resource() != nullptr) {
                Release();
            }
        }

        constexpr D3D12DepthBuffer& operator=(D3D12DepthBuffer&& o)
        {
            assert(this != &o);
            if (this != &o) {
                texture_ = std::move(o.texture_);
                dsv_ = o.dsv_;
                o.dsv_ = {};
            }
            return *this;
        }

        void Release();

#pragma region Getter
        [[nodiscard]] constexpr D3D12_CPU_DESCRIPTOR_HANDLE dsv() const { return dsv_.cpu; }
        [[nodiscard]] constexpr DescriptorHandle srv() const { return texture_.srv(); }
        [[nodiscard]] constexpr ID3D12Resource* const resource() const { return texture_.resource(); }
#pragma endregion //Getter
    private:
        D3D12Texture      texture_{};
        DescriptorHandle   dsv_{};
    };
}
