#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Window.h"
//#include "..\Graphics\Renderer.h"
#include "TestRenderer.h"

Platform::Window _windows[4];

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        bool all_closed{ true };
        for (u32 i{ 0 }; i < _countof(_windows); ++i)
        {
            if (_windows[i].IsClosed()) continue;

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
    Platform::WindowInitInfo info[]
    {
        {&win_proc, nullptr, L"Test window 1", /*100 - 2000*/1920 / 2 - 400 / 2, /*100 - 700 */ 1080 / 2 - 800 / 2, 400, 800},
        {&win_proc, nullptr, L"Test window 2", /*150 - 2000*/1920 / 2 - 800 / 2, /*150 - 700 */ 1080 / 2 - 400 / 2, 800, 400},
        {&win_proc, nullptr, L"Test window 3", /*200 - 2000*/1920 / 2 - 400 / 2, /*200 - 700 */ 1080 / 2 - 400 / 2, 400, 400},
        {&win_proc, nullptr, L"Test window 4", /*250 - 2000*/1920 / 2 - 800 / 2, /*250 - 700 */ 1080 / 2 - 600 / 2, 800, 600},
    };
    static_assert(_countof(info) == _countof(_windows));

    for (u32 i{ 0 }; i < _countof(_windows); ++i) {
        _windows[i] = Platform::GenerateWindow(&info[i]);
    }

    return true;
}

void TestRenderer::Run()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void TestRenderer::Shutdown()
{
    for (u32 i{ 0 }; i < _countof(_windows); ++i)
    {
        Platform::RemoveWindow(_windows[i].GetId());
    }
}
