

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
#include <crtdbg.h>
#endif // _DEBUG

#include <Windows.h>
#include "TestRenderer.h"
//#include "Platform/Window.h"

#include <filesystem>
// TODO: duplicate!
std::filesystem::path SetCurrentDirectoryToExecutablePath()
{
    // set the working directory to the executable path
    wchar_t path[MAX_PATH];
    const uint32_t length{ GetModuleFileName(0, &path[0], MAX_PATH) };
    if (!length || GetLastError() == ERROR_INSUFFICIENT_BUFFER) return {};
    std::filesystem::path p{ path };
    std::filesystem::current_path(p.parent_path());
    return std::filesystem::current_path();
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#if _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif // _DEBUG
    SetCurrentDirectoryToExecutablePath();

    TestRenderer test{};
    if (test.Initialize())
    {
        MSG msg{};
        bool is_running{ true };
        while (is_running)
        {
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                is_running &= (msg.message != WM_QUIT);
            }

            test.Run();
        }
    }
    test.Shutdown();

    return 0;
}
