#pragma once

#include "D3D12CommonHeaders.h"
#include "../Platform/Window.h"
#include "D3D12DescriptorHeap.h"
namespace RHI::D3D12::Parts {

    class D3D12SwapChain {
    public:
        explicit D3D12SwapChain(::Platform::Window window)
            : window_{ window }
        {
            assert(window_.Handle());
        }
#if USE_STL_VECTOR
        DISABLE_COPY(D3D12SwapChain);
        constexpr D3D12SwapChain(D3D12SwapChain&& o) :
            swap_chain_(o.swap_chain_),
            window_{ o.window_ },
            current_bb_index_{ o.current_bb_index_ },
            viewport_{ o.viewport_ },
            scissor_rect_{ o.scissor_rect_ },
            allow_tearing_{ o.allow_tearing_ },
            present_flags_{ o.present_flags_ }
        {
            for (u32 i{ 0 }; i < buffer_count; ++i)
            {
                render_target_data_[i].resource = o.render_target_data_[i].resource;
                render_target_data_[i].rtv = o.render_target_data_[i].rtv;
            }

            o.Reset();
        }

        constexpr D3D12SwapChain& operator=(D3D12SwapChain&& o)
        {
            assert(this != &o);
            if (this != &o)
            {
                Release();
                Move(o);
            }

            return *this;
        }

#endif // USE_STL_VECTOR

        ~D3D12SwapChain() { Release(); }
        void CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format);
        void Present() const;
        void Resize();

    private:
        void Finalize();
        void Release();

#if USE_STL_VECTOR
        constexpr void Move(D3D12SwapChain& o)
        {
            swap_chain_ = o.swap_chain_;
            for (u32 i{ 0 }; i < buffer_count; ++i)
            {
                render_target_data_[i] = o.render_target_data_[i];
            }
            window_ = o.window_;
            current_bb_index_ = o.current_bb_index_;
            allow_tearing_ = o.allow_tearing_;
            present_flags_ = o.present_flags_;
            viewport_ = o.viewport_;
            scissor_rect_ = o.scissor_rect_;

            o.Reset();
        }

        constexpr void Reset()
        {
            swap_chain_ = nullptr;
            for (u32 i{ 0 }; i < frame_buffer_count; ++i)
            {
                render_target_data_[i] = {};
            }
            window_ = {};
            current_bb_index_ = 0;
            allow_tearing_ = 0;
            present_flags_ = 0;
            viewport_ = {};
            scissor_rect_ = {};
        }
#endif

    public:
#pragma region Getter
        constexpr u32 Width() const { return (u32)viewport_.Width; }
        constexpr u32 Height() const { return (u32)viewport_.Height; }
#pragma endregion //Getter


    private:

        struct RenderTargetData
        {
            ID3D12Resource* resource{ nullptr };
            DescriptorHandle rtv{};
        };

        IDXGISwapChain4* swap_chain_{};
        RenderTargetData render_target_data_[buffer_count]{};
        ::Platform::Window window_{};
        mutable u32         current_bb_index_{ 0 };
        u32                 allow_tearing_{ 0 };
        u32                 present_flags_{ 0 };
        D3D12_VIEWPORT      viewport_{};
        D3D12_RECT          scissor_rect_{};

    };

}