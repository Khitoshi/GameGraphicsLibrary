#pragma once
#include "D3D12CommonHeaders.h"
#include "D3D12DescriptorHeap.h"

namespace RHI::D3D12::Parts {
    struct D3D12TextureInitInfo
    {
        ID3D12Heap1* heap{ nullptr };
        ID3D12Resource* resource{ nullptr };
        D3D12_SHADER_RESOURCE_VIEW_DESC* srv_desc{ nullptr };
        D3D12_RESOURCE_DESC* desc{ nullptr };
        D3D12_RESOURCE_ALLOCATION_INFO1     allocation_info{};
        D3D12_RESOURCE_STATES               initial_state{};
        D3D12_CLEAR_VALUE                   clear_value{};
    };

    class D3D12Texture
    {
    public:
        constexpr static u32 MAXMIPS{ 14 }; // support up to 16k resolutions.
        D3D12Texture() = default;
        explicit D3D12Texture(D3D12TextureInitInfo info);
        DISABLE_COPY(D3D12Texture);
        constexpr D3D12Texture(D3D12Texture&& o) :
            resource_{ o.resource_ },
            srv_{ o.srv_ }
        {
            o.Reset();
        }

        constexpr D3D12Texture& operator=(D3D12Texture&& o)
        {
            assert(this != &o);
            if (this != &o)
            {
                Release();
                Move(o);
            }
            return *this;
        }

        ~D3D12Texture() { Release(); }

        void Release();
#pragma region Getter
        [[nodiscard]] constexpr ID3D12Resource* const resource() const { return resource_; }
        [[nodiscard]] constexpr DescriptorHandle srv() const { return srv_; }
#pragma endregion //Getter


    private:
        constexpr void Move(D3D12Texture& o)
        {
            resource_ = o.resource_;
            srv_ = o.srv_;
            o.Reset();
        }

        constexpr void Reset()
        {
            resource_ = nullptr;
            srv_ = {};
        }

        ID3D12Resource* resource_{ nullptr };
        DescriptorHandle    srv_;
    };
}
