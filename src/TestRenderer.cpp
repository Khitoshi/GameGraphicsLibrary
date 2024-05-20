#include "..\Platform\PlatformTypes.h"
#include "..\Platform\Window.h"
//#include "..\Graphics\Renderer.h"
#include "TestRenderer.h"
#include "Renderer/Renderer.h"
#include "../Renderer/Timer.h"
#include "ShaderCompilation.h"
#include "../EngineAPI/GameEntity.h"
#include "../EngineAPI/Camera.h"
#include "Components/Transform.h"
#include "Components/Entity.h"
#include "RHI/DirectX12/D3D12Camera.h"


GameEntity::Entity entity{};
Renderer::RenderSurface _surfaces[1];
Graphics::Camera camera{};

Time timer{};
bool resized{ false };
bool is_restarting{ false };

void DestroyRenderSurface(Renderer::RenderSurface&);
bool TestInitialize();
void TestShutdown();

LRESULT win_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    bool toggle_fullscreen{ false };

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
        if (all_closed && !toggle_fullscreen) {
            PostQuitMessage(0);
            return 0;
        }
        break;
    }
    case WM_SIZE:
    {
        resized = (wparam != SIZE_MINIMIZED);
        break;

    }
    case WM_SYSCHAR:
    {
        //if (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN)) {
        //    Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
        //    win.SetFullscreen(!win.IsFullscreen());
        //    return 0;
        //}
        toggle_fullscreen = (wparam == VK_RETURN && (HIWORD(lparam) & KF_ALTDOWN));
        break;
    }
    case WM_KEYDOWN:
    {
        if (wparam == VK_ESCAPE) {
            PostMessage(hwnd, WM_CLOSE, 0, 0);
            return 0;
        }
        else if (wparam == VK_F11) {
            is_restarting = true;
            TestShutdown();
            TestInitialize();
        }
    }
    }

    if ((resized && GetAsyncKeyState(VK_LBUTTON) >= 0) || toggle_fullscreen)
    {
        Platform::Window win{ Platform::window_id{(Id::id_type)GetWindowLongPtr(hwnd, GWLP_USERDATA)} };
        for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
        {
            if (win.GetId() == _surfaces[i].window.GetId())
            {
                if (toggle_fullscreen)
                {
                    win.SetFullscreen(!win.IsFullscreen());
                    // The default window procedure will play a system notification sound
                    // when pressing the Alt+Enter keyboard combination if WM_SYSCHAR is
                    // not handled. By returning 0 we can tell the system that we handled
                    // this message.
                    return 0;
                }
                else
                {
                    _surfaces[i].surface.Resize(win.Width(), win.Height());
                    resized = false;
                }
                break;
            }
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

GameEntity::Entity CreateOneGameEntity()
{
    Transform::InitInfo transform_info{};
    Math::v3a rot{ 0, 3.14f, 0 };
    DirectX::XMVECTOR quat{ DirectX::XMQuaternionRotationRollPitchYawFromVector(DirectX::XMLoadFloat3A(&rot)) };
    Math::v4a rot_quat;
    DirectX::XMStoreFloat4A(&rot_quat, quat);
    memcpy(&transform_info.rotation[0], &rot_quat.x, sizeof(transform_info.rotation));

    GameEntity::EntityInfo entity_info{};
    entity_info.transform = &transform_info;
    GameEntity::Entity ntt{ GameEntity::Create(entity_info) };
    assert(ntt.IsValid());
    return ntt;
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


//bool TestRenderer::Initialize()
bool TestInitialize()
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

    is_restarting = false;

    entity = CreateOneGameEntity();
    camera = Renderer::CreateCamera(RHI::D3D12::Camera::PerspectiveCameraInitInfo(entity.GetID()));
    assert(camera.IsValid());

    return true;
}

void TestShutdown()
{
    if (camera.IsValid()) {
        Renderer::RemoveCamera(camera.GetID());
    }

    if (entity.IsValid()) {
        GameEntity::Remove(entity.GetID());
    }

    for (auto& surface : _surfaces) {
        DestroyRenderSurface(surface);
    }

    Renderer::Shutdown();
}

bool TestRenderer::Initialize()
{
    return TestInitialize();
}

void TestRenderer::Run()
{
    timer.Begin();

    for (u32 i{ 0 }; i < _countof(_surfaces); ++i)
    {
        if (_surfaces[i].surface.is_valid()) {
            _surfaces[i].surface.Render();
        }
    }

    timer.End();
}

void TestRenderer::Shutdown()
{
    TestShutdown();
}



//TODO: assetフォルダーを作成し、fbxファイルを格納そして、fbxファイルを全て読み込む処理を作成する shaderの読み込みを参考にして