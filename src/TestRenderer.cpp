#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Window.h"
//#include "..\Graphics\Renderer.h"
#include "TestRenderer.h"
#include "Renderer/Renderer.h"
#include "../Renderer/Timer.h"
#include "ShaderCompilation.h"

//Platform::Window _windows[4];
//Renderer::RenderSurface _surfaces[4];
Renderer::RenderSurface _surfaces[1];

Time timer{};

void DestroyRenderSurface(Renderer::RenderSurface&);

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        bool all_closed{ true };
        for (u32 i{ 0 }; i < _countof(_surfaces); ++i) {
            if (_surfaces[i].window.IsValid()) {
                if (_surfaces[i].window.IsClosed()) {
                    DestroyRenderSurface(_surfaces[i]);
                }
                else {
                    all_closed = false;
                }
            }
        }
        if (all_closed) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    case WM_SYSCHAR:
        if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN)) {
            Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
            win.SetFullscreen(!win.IsFullscreen());
            return 0;
        }
        break;
    case WM_KEYDOWN:
        if (wparam == VK_ESCAPE) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

void CreateRenderSurface(Renderer::RenderSurface& surface, Platform::WindowInitInfo info)
{
    surface.window = Platform::GenerateWindow(&info);
    surface.surface = Renderer::CreateSurface(surface.window);
}

void DestroyRenderSurface(Renderer::RenderSurface& surface)
{
    Renderer::RenderSurface temp{ surface };
    surface = {};

    if (temp.surface.is_valid())Renderer::RemoveSurface(temp.surface.get_id());
    if (temp.surface.is_valid())Platform::RemoveWindow(temp.window.GetId());
}


bool TestRenderer::Initialize()
{

    while (!CompileShaders())
    {
        // Pop up a message box allowing the user to retry compilation.
        if (MessageBox(nullptr, L"Failed to compile engine shaders.", L"Shader Compilation Error", MB_RETRYCANCEL) != IDRETRY)
            return false;
    }
    if (!Renderer::Initialize(Renderer::GRAPHICS_PLATFORM::direct3d12)) return false;

    //bool result{ Renderer::Initialize(Renderer::GRAPHICS_PLATFORM::direct3d12) };
    //if (!result) return result;

    Platform::WindowInitInfo info[]
    {
        //{&win_proc, nullptr, L"Renderer window 1", 1920 / 2 - 400 / 2, 1080 / 2 - 800 / 2, 400, 800},
        //{&win_proc, nullptr, L"Renderer window 2", 1920 / 2 - 800 / 2, 1080 / 2 - 400 / 2, 800, 400},
        //{&win_proc, nullptr, L"Renderer window 3", 1920 / 2 - 400 / 2, 1080 / 2 - 400 / 2, 400, 400},
        {&win_proc, nullptr, L"Renderer window 4", 1920 / 2 - 800 / 2, 1080 / 2 - 600 / 2, 800, 600},
    };
    static_assert(_countof(info) == _countof(_surfaces));

    for (u32 i{ 0 }; i < _countof(_surfaces); ++i) {
        CreateRenderSurface(_surfaces[i], info[i]);
    }

    return true;
}

void TestRenderer::Run()
{
    timer.Begin();

    for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
    {
        if (_surfaces[i].surface.is_valid())
        {
            _surfaces[i].surface.Render();
        }
    }

    timer.End();
}

void TestRenderer::Shutdown()
{
    for (auto& surface : _surfaces) {
        DestroyRenderSurface(surface);
    }

    Renderer::Shutdown();
}
