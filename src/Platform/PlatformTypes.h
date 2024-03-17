#pragma once

#include "CommonHeaders.h"
#include <Windows.h>

namespace Platform {

    using window_proc = LRESULT(*)(HWND, UINT, WPARAM, LPARAM);
    using window_handle = HWND;

    struct WindowInitInfo
    {
        window_proc     callback{ nullptr };
        window_handle   parent{ nullptr };
        const wchar_t* caption{ nullptr };
        s32             left{ 0 };
        s32             top{ 0 };
        s32             width{ 1920 };
        s32             height{ 1080 };
    };
}// namespace Platform
