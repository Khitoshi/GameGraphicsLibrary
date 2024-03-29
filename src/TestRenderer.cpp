#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Window.h"
//#include "..\Graphics\Renderer.h"
#include "TestRenderer.h"
#include "Renderer/Renderer.h"

//Platform::Window _windows[4];
Renderer::RenderSurface _surfaces[4];

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        bool all_closed{ true };
        for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
        {
            if (_surfaces[i].window.IsClosed()) continue;

            all_closed = false;
        }
        if (all_closed)
        {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    case WM_SYSCHAR:
        if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN))
        {
            Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
            win.SetFullscreen(!win.IsFullscreen());
            return 0;
        }
        break;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}


bool TestRenderer::Initialize()
{
    bool result{ Renderer::Initialize(Renderer::GRAPHICS_PLATFORM::direct3d12) };
    if (!result) return result;

    Platform::WindowInitInfo info[]
    {
        {&win_proc, nullptr, L"Renderer window 1", 1920 / 2 - 400 / 2, 1080 / 2 - 800 / 2, 400, 800},
        {&win_proc, nullptr, L"Renderer window 2", 1920 / 2 - 800 / 2, 1080 / 2 - 400 / 2, 800, 400},
        {&win_proc, nullptr, L"Renderer window 3", 1920 / 2 - 400 / 2, 1080 / 2 - 400 / 2, 400, 400},
        {&win_proc, nullptr, L"Renderer window 4", 1920 / 2 - 800 / 2, 1080 / 2 - 600 / 2, 800, 600},
    };
    static_assert(_countof(info) == _countof(_surfaces));

    for (u32 i{ 0 }; i < _countof(_surfaces); ++i) {
        _surfaces[i].window = Platform::GenerateWindow(&info[i]);
    }

    return true;
}

void TestRenderer::Run()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    Renderer::Render();
}

void TestRenderer::Shutdown()
{
    for (auto& surface : _surfaces)
    {
        Platform::RemoveWindow(surface.window.GetId());
    }

    Renderer::Shutdown();
}
