#pragma once

#include "CommonHeaders.h"

#include <dxgi1_6.h>
#include <d3d12.h>
#include <wrl.h>
#include <mutex> 

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

namespace RHI::D3D12 {
    using Microsoft::WRL::ComPtr;

    using std::shared_ptr;
    using std::make_shared;
}

namespace RHI::D3D12::Parts {
    constexpr u32 frame_buffer_count{ 3 };
    constexpr u32 buffer_count{ 3 };
    //using Microsoft::WRL::ComPtr;
}

#ifdef _DEBUG
#ifndef DXCall
#define DXCall(x)                           \
if(FAILED(x)) {                             \
    char line_number[32];                   \
    sprintf_s(line_number, "%u", __LINE__); \
    OutputDebugStringA("Error in: ");       \
    OutputDebugStringA(__FILE__);           \
    OutputDebugStringA("\nLine: ");         \
    OutputDebugStringA(line_number);        \
    OutputDebugStringA("\n");               \
    OutputDebugStringA(#x);                 \
    OutputDebugStringA("\n");               \
    __debugbreak();                         \
}
#endif // !DXCall
#else
#ifndef DXCall
#define DXCall(x) x
#endif // !DXCall
#endif // _DEBUG


#ifdef _DEBUG
// COMオブジェクトの名前を設定し、VisualStudioの出力パネルにデバッグ文字列を出力する
#define NAME_D3D12_OBJECT(obj, name)                \
obj->SetName(name);                                 \
OutputDebugString(L"::D3D12 Object Created: ");     \
OutputDebugString(name); OutputDebugString(L"\n");  

// COMオブジェクトの名前を設定し、VisualStudioの出力パネルにデバッグインデックス付き文字列を出力する
#define NAME_D3D12_OBJECT_INDEXED(obj, n, name)     \
{                                                   \
wchar_t full_name[128];                             \
if (swprintf_s(full_name, L"%s[%u]", name, n) > 0 ){\
    obj->SetName(full_name);                        \
    OutputDebugString(L"::D3D12 Object Created: "); \
    OutputDebugString(full_name);                   \
    OutputDebugString(L"\n");                       \
}}

#else
#define NAME_D3D12_OBJECT(x, name)
#define NAME_D3D12_OBJECT_INDEXED(x, n, name)
#endif // _DEBUG