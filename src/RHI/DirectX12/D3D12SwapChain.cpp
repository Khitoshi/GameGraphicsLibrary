#include "D3D12SwapChain.h"
#include "D3D12Core.h"

namespace RHI::D3D12::Parts {
    namespace {
        constexpr DXGI_FORMAT ToNon_SRGB(DXGI_FORMAT format)
        {
            if (format == DXGI_FORMAT_R8G8B8A8_UNORM_SRGB) return DXGI_FORMAT_R8G8B8A8_UNORM;

            return format;
        }
    }

    void D3D12SwapChain::CreateSwapChain(IDXGIFactory7* factory, ID3D12CommandQueue* cmd_queue, DXGI_FORMAT format)
    {
        assert(factory && cmd_queue);
        Release();

        if (SUCCEEDED(factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allow_tearing_, sizeof(u32))) && allow_tearing_) {
            present_flags_ = DXGI_PRESENT_ALLOW_TEARING;
        }

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        desc.BufferCount = frame_buffer_count;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.Flags = allow_tearing_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
        desc.Format = ToNon_SRGB(format);
        desc.Height = window_.Height();
        desc.Width = window_.Width();
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Scaling = DXGI_SCALING_STRETCH;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.Stereo = false;

        IDXGISwapChain1* swap_chain;
        HWND hwnd{ (HWND)window_.Handle() };
        DXCall(factory->CreateSwapChainForHwnd(cmd_queue, hwnd, &desc, nullptr, nullptr, &swap_chain));
        DXCall(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER));
        DXCall(swap_chain->QueryInterface(IID_PPV_ARGS(&swap_chain_)));
        //core::release(swap_chain);
        swap_chain->Release();

        current_bb_index_ = swap_chain_->GetCurrentBackBufferIndex();

        for (u32 i{ 0 }; i < buffer_count; ++i) {
            render_target_data_[i].rtv = Core::GetRTVHeap().Allocate();
        }

        Finalize();
    }

    void D3D12SwapChain::Present() const
    {
        assert(swap_chain_);
        DXCall(swap_chain_->Present(0, present_flags_));
        current_bb_index_ = swap_chain_->GetCurrentBackBufferIndex();
    }

    void D3D12SwapChain::Resize()
    {}

    void D3D12SwapChain::Finalize()
    {
        // create RTVs for back-buffers
        for (u32 i{ 0 }; i < buffer_count; ++i)
        {
            RenderTargetData& data{ render_target_data_[i] };
            assert(!data.resource);
            DXCall(swap_chain_->GetBuffer(i, IID_PPV_ARGS(&data.resource)));
            D3D12_RENDER_TARGET_VIEW_DESC desc{};
            desc.Format = Core::GetDefaultRenderTargetFormat();
            desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
            Core::GetDevice()->CreateRenderTargetView(data.resource, &desc, data.rtv.cpu);
        }

        DXGI_SWAP_CHAIN_DESC desc{};
        DXCall(swap_chain_->GetDesc(&desc));
        const u32 width{ desc.BufferDesc.Width };
        const u32 height{ desc.BufferDesc.Height };
        assert(window_.Width() == width);
        assert(window_.Height() == height);

        // set viewport and scissor rect
        viewport_.TopLeftX = 0.f;
        viewport_.TopLeftY = 0.f;
        viewport_.Width = (float)width;
        viewport_.Height = (float)height;
        viewport_.MinDepth = 0.f;
        viewport_.MaxDepth = 1.f;

        scissor_rect_ = { 0, 0, (s32)width, (s32)height };
    }

    void D3D12SwapChain::Release()
    {
        for (u32 i{ 0 }; i < buffer_count; ++i)
        {
            RenderTargetData& data{ render_target_data_[i] };
            if (data.resource) {
                data.resource->Release();
                data.resource = nullptr;
            }

            Core::GetRTVHeap().Free(data.rtv);
        }

        if (swap_chain_) {
            swap_chain_->Release();
            swap_chain_ = nullptr;
        }
    }

}

