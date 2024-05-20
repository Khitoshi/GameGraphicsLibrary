#pragma once

#include "CommonHeaders.h"
#include "../Renderer/Renderer.h"
#include "../RHI/DirectX12/D3D12Camera.h"

namespace RHI {

    namespace
    {
        using Surface = ::Renderer::Surface;
        using SurfaceID = ::Renderer::SurfaceID;
    }

    struct RHIPlatformInterface
    {
        bool(*Initialize)(void);
        void(*Shutdown)(void);

        struct {
            Surface(*Create)(::Platform::Window);
            void(*Remove)(SurfaceID);
            void(*Resize)(SurfaceID, u32, u32);
            u32(*Width)(SurfaceID);
            u32(*Height)(SurfaceID);
            void(*Render)(SurfaceID);
        } surface;

        struct {
            ::Graphics::Camera(*Create)(RHI::D3D12::Camera::CameraInitInfo);
            void(*Remove)(Graphics::CameraID);
            void(*SetParameter)(Graphics::CameraID, RHI::D3D12::Camera::CameraParameter::PARAMETER, const void* const, u32);
            void(*GetParameter)(Graphics::CameraID, RHI::D3D12::Camera::CameraParameter::PARAMETER, void* const, u32);
        } camera;

        Renderer::GRAPHICS_PLATFORM platform = (Renderer::GRAPHICS_PLATFORM)-1;
    };
}

