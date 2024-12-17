#include "Renderer.h"
#include "../RHI/RHIPlatformInterface.h"
#include "../RHI/DirectX12/D3D12Interface.h"

namespace Renderer
{
    namespace {
        RHI::RHIPlatformInterface gfx{};

        bool SetPlatformInterface(GRAPHICS_PLATFORM platform)
        {
            switch (platform)
            {
            case GRAPHICS_PLATFORM::direct3d12:
                RHI::D3D12::GetPlatformInterface(gfx);
                break;
            default:
                return false;
            }
            assert(gfx.platform == platform);
            return true;
        }

        // サポートされているAPIごとに、コンパイルされたエンジンシェーダーファイルの場所を定義
        constexpr const char* engine_shader_paths[]{
            ".\\Shaders\\D3D12\\shaders.bin",
        };

        //
        constexpr const char* engine_models_paths = ".\\Models\\models.bin";

    }

    bool Initialize(GRAPHICS_PLATFORM platform)
    {
        return SetPlatformInterface(platform) && gfx.Initialize();
    }

    void Shutdown()
    {
        if (gfx.platform != (GRAPHICS_PLATFORM)-1) gfx.Shutdown();
    }

    Surface CreateSurface(Platform::Window window)
    {
        return gfx.surface.Create(window);
    }

    void RemoveSurface(SurfaceID id)
    {
        assert(Id::IsValid(id));
        gfx.surface.Remove(id);
    }

    ::Graphics::Camera CreateCamera(::RHI::D3D12::Camera::CameraInitInfo info)
    {
        return gfx.camera.Create(info);
    }

    void RemoveCamera(::Graphics::CameraID id)
    {
        gfx.camera.Remove(id);
    }

#pragma region Getter
    void Surface::Resize(u32 width, u32 height) const
    {
        assert(is_valid());
        gfx.surface.Resize(id_, width, height);
    }

    u32 Surface::Width() const
    {
        assert(is_valid());
        return gfx.surface.Width(id_);
    }

    u32 Surface::Height() const
    {
        assert(is_valid());
        return gfx.surface.Height(id_);
    }

    void Surface::Render() const
    {
        assert(is_valid());
        return gfx.surface.Render(id_);
    }

    const char* GetEngineShadersPath()
    {
        return engine_shader_paths[(u32)gfx.platform];
    }

    const char* GetEngineShadersPath(GRAPHICS_PLATFORM platform)
    {
        return engine_shader_paths[(u32)platform];
    }

    const char* GetEngineModelsPath()
    {
        return engine_models_paths;
    }

    const RHI::RHIPlatformInterface* GetPlatformInterface()
    {
        return &gfx;
    }
#pragma endregion //Getter

}

