#include "D3D12Gpass.h"
#include "D3D12Core.h"
#include "D3D12Shaders.h"
#include "D3D12RenderTexture.h"
#include "D3D12DepthBuffer.h"

namespace RHI::D3D12::GPASS
{
    namespace {
        struct GPassRootParamIndices {
            enum : u32 {
                ROOT_CONSTANTS,

                COUNT
            };
        };


        constexpr DXGI_FORMAT       main_buffer_format{ DXGI_FORMAT_R16G16B16A16_FLOAT };
        constexpr DXGI_FORMAT       depth_buffer_format{ DXGI_FORMAT_D32_FLOAT };
        constexpr Math::u32v2       initial_dimensions{ 100, 100 };

        Parts::D3D12RenderTexture       gpass_main_buffer{};
        Parts::D3D12DepthBuffer         gpass_depth_buffer{};
        Math::u32v2                     dimensions{ initial_dimensions };
        D3D12_RESOURCE_BARRIER_FLAGS    flags{};

        ID3D12RootSignature* gpass_root_sig{ nullptr };
        ID3D12PipelineState* gpass_pso{ nullptr };

#if _DEBUG
        constexpr f32                   clear_value[4]{ 0.5f, 0.5f, 0.5f, 1.f };
#else
        constexpr f32                   clear_value[4]{ };
#endif

        bool CreateBuffers(Math::u32v2 size)
        {
            assert(size.x && size.y);
            gpass_main_buffer.Release();
            gpass_depth_buffer.Release();

            D3D12_RESOURCE_DESC desc{};
            desc.Alignment = 0; // 注：0は64KB（MSAAの場合は4MB）と同じ
            desc.DepthOrArraySize = 1;
            desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
            desc.Format = main_buffer_format;
            desc.Height = size.y;
            desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
            desc.MipLevels = 0; // すべてのMipLevelsのためのスペースを作る
            desc.SampleDesc = { 1, 0 };
            desc.Width = size.x;

            // Mainバッファの作成
            {
                Parts::D3D12TextureInitInfo info{};
                info.desc = &desc;
                info.initial_state = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
                info.clear_value.Format = desc.Format;
                memcpy(&info.clear_value.Color, &clear_value[0], sizeof(clear_value));
                gpass_main_buffer = Parts::D3D12RenderTexture{ info };
            }

            desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
            desc.Format = depth_buffer_format;
            desc.MipLevels = 1;

            // 深度バッファーの作成
            {
                Parts::D3D12TextureInitInfo info{};
                info.desc = &desc;
                info.initial_state = D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
                info.clear_value.Format = desc.Format;
                info.clear_value.DepthStencil.Depth = 0.f;
                info.clear_value.DepthStencil.Stencil = 0;

                gpass_depth_buffer = Parts::D3D12DepthBuffer{ info };
            }

            NAME_D3D12_OBJECT(gpass_main_buffer.Resource(), L"GPass Main Buffer");
            NAME_D3D12_OBJECT(gpass_depth_buffer.resource(), L"GPass Depth Buffer");

            flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;

            return gpass_main_buffer.Resource() && gpass_depth_buffer.resource();
        }

        bool CreateGpassPSOAndRootSignature()
        {
            assert(!gpass_root_sig && !gpass_pso);

            // GPassRootSignature 作成
            using idx = GPassRootParamIndices;
            D3DX::D3D12RootParameter parameters[idx::COUNT]{};
            parameters[0].AsConstants(3, D3D12_SHADER_VISIBILITY_PIXEL, 1);
            const D3DX::D3D12RootSignatureDesc root_signature{ &parameters[0], idx::COUNT };
            gpass_root_sig = root_signature.create();
            assert(gpass_root_sig);
            NAME_D3D12_OBJECT(gpass_root_sig, L"GPass Root Signature");

            // GPassPSO 作成
            struct {
                D3DX::D3D12PipelineStateSubobject_RootSignature         root_signature{ gpass_root_sig };
                D3DX::D3D12PipelineStateSubobject_VS                    vs{ Shaders::GetEngineShader(Shaders::EngineShader::FULLSCREENTRIANGLE_VS) };
                D3DX::D3D12PipelineStateSubobject_PS                    ps{ Shaders::GetEngineShader(Shaders::EngineShader::FILLCOLOR_PS) };
                D3DX::D3D12PipelineStateSubobject_PrimitiveTopology     primitive_topology{ D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE };
                D3DX::D3D12PipelineStateSubobject_RenderTargetFormats   render_target_formats;
                D3DX::D3D12PipelineStateSubobject_DepthStencilFormat    depth_stencil_format{ depth_buffer_format };
                D3DX::D3D12PipelineStateSubobject_Rasterizer            rasterizer{ D3DX::rasterizer_state.NoCull };
                D3DX::D3D12PipelineStateSubobject_DepthStencil1         depth{ D3DX::depth_state.Disabled };
            } stream;

            D3D12_RT_FORMAT_ARRAY rtf_array{};
            rtf_array.NumRenderTargets = 1;
            rtf_array.RTFormats[0] = main_buffer_format;

            stream.render_target_formats = rtf_array;

            gpass_pso = D3DX::CreatePipelineState(&stream, sizeof(stream));
            NAME_D3D12_OBJECT(gpass_pso, L"GPass Pipeline State Object");

            return gpass_root_sig && gpass_pso;
        }
    }

    bool Initialize()
    {
        return CreateBuffers(initial_dimensions) && CreateGpassPSOAndRootSignature();
    }

    void Shutdown()
    {
        gpass_main_buffer.Release();
        gpass_depth_buffer.Release();
        dimensions = initial_dimensions;

        //Core::release(gpass_root_sig);
        if (gpass_root_sig) {
            gpass_root_sig->Release();
            gpass_root_sig = nullptr;
        }

        //Core::release(gpass_pso);
        if (gpass_pso) {
            gpass_pso->Release();
            gpass_pso = nullptr;
        }
    }

    void SetSize(Math::u32v2 size)
    {
        Math::u32v2& d{ dimensions };
        if (size.x > d.x || size.y > d.y)
        {
            d = { std::max(size.x, d.x), std::max(size.y, d.y) };
            CreateBuffers(d);
        }
    }

    void DepthPrepass(ID3D12GraphicsCommandList6* cmd_list, const Parts::FrameInfo& info)
    {
    }

    void Render(ID3D12GraphicsCommandList6* cmd_list, const Parts::FrameInfo& info)
    {
        cmd_list->SetGraphicsRootSignature(gpass_root_sig);
        cmd_list->SetPipelineState(gpass_pso);

        static u32 frame{ 0 };
        struct {
            f32 width;
            f32 height;
            u32 frame;
        } constants{ (f32)info.SurfaceWidth, (f32)info.SurfaceHeight, ++frame };

        using idx = GPassRootParamIndices;
        cmd_list->SetGraphicsRoot32BitConstants(idx::ROOT_CONSTANTS, 3, &constants, 0);

        cmd_list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd_list->DrawInstanced(3, 1, 0, 0);
    }

    void AddTransitionsForDepthPrepass(D3DX::ResourceBarrier& barriers)
    {
        barriers.Add(
            gpass_main_buffer.Resource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_BARRIER_FLAG_BEGIN_ONLY
        );

        barriers.Add(
            gpass_depth_buffer.resource(),
            D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_DEPTH_WRITE
        );
    }

    void AddTransitionsForGpass(D3DX::ResourceBarrier& barriers)
    {
        barriers.Add(gpass_main_buffer.Resource(),
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_BARRIER_FLAG_END_ONLY
        );

        barriers.Add(gpass_depth_buffer.resource(),
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            D3D12_RESOURCE_STATE_DEPTH_READ | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
    }

    void AddTransitionsForPostProcess(D3DX::ResourceBarrier& barriers)
    {
        barriers.Add(gpass_main_buffer.Resource(),
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    void SetRenderTargetsForDepthPrepass(ID3D12GraphicsCommandList6* cmd_list)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ gpass_depth_buffer.dsv() };
        cmd_list->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0.f, 0, 0, nullptr);
        cmd_list->OMSetRenderTargets(0, nullptr, 0, &dsv);
    }

    void SetRenderTargetsForGpass(ID3D12GraphicsCommandList6* cmd_list)
    {
        const D3D12_CPU_DESCRIPTOR_HANDLE rtv{ gpass_main_buffer.RTV(0) };
        const D3D12_CPU_DESCRIPTOR_HANDLE dsv{ gpass_depth_buffer.dsv() };

        cmd_list->ClearRenderTargetView(rtv, clear_value, 0, nullptr);
        cmd_list->OMSetRenderTargets(1, &rtv, 0, &dsv);
    }

#pragma region Getter
    const  Parts::D3D12RenderTexture& MainBuffer()
    {
        return gpass_main_buffer;
    }

    const  Parts::D3D12DepthBuffer& DepthBuffer()
    {
        return gpass_depth_buffer;
    }
#pragma endregion //Getter
}

