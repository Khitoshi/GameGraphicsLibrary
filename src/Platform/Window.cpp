#include "Window.h"


namespace Platform {
    namespace {
        struct WindowInfo
        {
            HWND    hwnd{ nullptr };
            RECT    client_area{ 0, 0, 1920, 1080 };
            RECT    fullscreen_area{};
            POINT   top_left{ 0, 0 };
            DWORD   style{ WS_VISIBLE };
            bool    is_fullscreen{ false };
            bool    is_closed{ false };
        };

        Container::vector<WindowInfo> windows;
        Container::vector<u32> available_slots;

#pragma region ForwardDeclaration
        WindowInfo& GetFromHandle(window_handle handle);

#pragma endregion //ForwardDeclaration

        u32 AddToWindows(WindowInfo info)
        {
            u32 id{ u32_invalid_id };
            if (available_slots.empty()) {
                id = (u32)windows.size();
                windows.emplace_back(info);
            }
            else {
                id = available_slots.back();
                available_slots.pop_back();
                assert(id != u32_invalid_id);
                windows[id] = info;
            }
            return id;
        }

        void RemoveFromWindows(u32 id)
        {
            assert(id < windows.size());
            available_slots.emplace_back(id);
        }

        WindowInfo& GetFromId(window_id id)
        {
            assert(id < windows.size());
            assert(windows[id].hwnd);
            return windows[id];
        }

        LRESULT CALLBACK InternalWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            WindowInfo* info{ nullptr };
            switch (msg)
            {
            case WM_DESTROY:
                GetFromHandle(hwnd).is_closed = true;
                break;
            case WM_EXITSIZEMOVE:
                info = &GetFromHandle(hwnd);
                break;
            case WM_SIZE:
                if (wparam == SIZE_MAXIMIZED)
                {
                    info = &GetFromHandle(hwnd);
                }
                break;
            case WM_SYSCOMMAND:
                if (wparam == SC_RESTORE)
                {
                    info = &GetFromHandle(hwnd);
                }
                break;
            default:
                break;
            }

            if (info)
            {
                assert(info->hwnd);
                GetClientRect(info->hwnd, info->is_fullscreen ? &info->fullscreen_area : &info->client_area);
            }

            LONG_PTR long_ptr{ GetWindowLongPtr(hwnd, 0) };
            return long_ptr
                ? ((window_proc)long_ptr)(hwnd, msg, wparam, lparam)
                : DefWindowProc(hwnd, msg, wparam, lparam);
        }

        void ResizeWindow(const WindowInfo& info, const RECT& area)
        {
            // Asjust the window size for correct device size
            RECT window_rect{ area };
            AdjustWindowRect(&window_rect, info.style, FALSE);

            const s32 width{ window_rect.right - window_rect.left };
            const s32 height{ window_rect.bottom - window_rect.top };

            MoveWindow(info.hwnd, info.top_left.x, info.top_left.y, width, height, true);
        }

        void ResizeWindow(window_id id, u32 width, u32 height)
        {
            WindowInfo& info{ GetFromId(id) };

            // NOTE: when we host the window in the level editor we just update
            //       the internal data (i.e. the client area dimensions).
            if (info.style & WS_CHILD)
            {
                GetClientRect(info.hwnd, &info.client_area);
            }
            else
            {
                // フルスクリーンモードでもリサイズを行うのは、ユーザーが画面解像度を 
                // 変更した場合に対応するため
                RECT& area{ info.is_fullscreen ? info.fullscreen_area : info.client_area };
                area.bottom = area.top + height;
                area.right = area.left + width;

                ResizeWindow(info, area);
            }
        }

#pragma region Getter
        WindowInfo& GetFromHandle(window_handle handle)
        {
            const window_id id{ (Id::id_type)GetWindowLongPtr(handle, GWLP_USERDATA) };
            return GetFromId(id);
        }

        bool IsWindowFullscreen(window_id id)
        {
            return GetFromId(id).is_fullscreen;
        }

        window_handle GetWindowHandle(window_id id)
        {
            return GetFromId(id).hwnd;
        }

        Math::u32v4 GetWindowSize(window_id id)
        {
            WindowInfo& info{ GetFromId(id) };
            RECT& area{ info.is_fullscreen ? info.fullscreen_area : info.client_area };
            return { (u32)area.left, (u32)area.top , (u32)area.right , (u32)area.bottom };
        }

        bool IsWindowClosed(window_id id)
        {
            return GetFromId(id).is_closed;
        }
#pragma endregion //Getter

#pragma region Setter
        void SetWindowCaption(window_id id, const wchar_t* caption)
        {
            WindowInfo& info{ GetFromId(id) };
            SetWindowText(info.hwnd, caption);
        }

        void SetWindowFullscreen(window_id id, bool is_fullscreen)
        {
            WindowInfo& info{ GetFromId(id) };
            if (info.is_fullscreen == is_fullscreen) return;

            info.is_fullscreen = is_fullscreen;

            if (is_fullscreen) {
                //フルスクリーン前のwindowの位置とサイズを保存して
                //フルスクリーン解除時に前のwindowの位置とサイズに戻せるようにする
                GetClientRect(info.hwnd, &info.client_area);
                RECT rect;
                GetWindowRect(info.hwnd, &rect);
                info.top_left.x = rect.left;
                info.top_left.y = rect.top;
                SetWindowLongPtr(info.hwnd, GWL_STYLE, 0);
            }
            else {
                SetWindowLongPtr(info.hwnd, GWL_STYLE, info.style);
                ResizeWindow(info, info.client_area);
            }

            ShowWindow(info.hwnd, SW_SHOWNORMAL);
        }


#pragma endregion //Setter

    }// anonymous namespace

    void Window::Resize(u32 width, u32 height) const
    {
        assert(IsValid());
        ResizeWindow(_id, width, height);
    }

#pragma region Getter
    bool Window::IsFullscreen() const
    {
        assert(IsValid());
        return IsWindowFullscreen(_id);
    }

    void* Window::Handle() const
    {
        assert(IsValid());
        return GetWindowHandle(_id);
    }

    Math::u32v4 Window::Size() const
    {
        assert(IsValid());
        return GetWindowSize(_id);
    }

    u32 Window::Width() const
    {
        Math::u32v4 s{ Size() };
        return s.z - s.x;
    }

    u32 Window::Height() const
    {
        Math::u32v4 s{ Size() };
        return s.w - s.y;
    }

    bool Window::IsClosed() const
    {
        assert(IsValid());
        return IsWindowClosed(_id);
    }

#pragma endregion

#pragma region Setter
    void Window::SetFullscreen(bool is_fullscreen) const
    {
        assert(IsValid());
        SetWindowFullscreen(_id, is_fullscreen);
    }

    void Window::SetCaption(const wchar_t* caption) const
    {
        assert(IsValid());
        SetWindowCaption(_id, caption);
    }

#pragma endregion

    Window GenerateWindow(const WindowInitInfo* const init_info)
    {
        window_proc callback{ init_info ? init_info->callback : nullptr };
        window_handle parent{ init_info ? init_info->parent : nullptr };

        // windowクラスの設定
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = InternalWindowProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = callback ? sizeof(callback) : 0;
        wc.hInstance = 0;
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = CreateSolidBrush(RGB(26, 48, 76));
        wc.lpszMenuName = NULL;
        wc.lpszClassName = L"GameGraphics";
        wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

        // windowクラスの登録
        RegisterClassEx(&wc);

        WindowInfo info{};
        info.client_area.right = (init_info && init_info->width) ? info.client_area.left + init_info->width : info.client_area.right;
        info.client_area.bottom = (init_info && init_info->height) ? info.client_area.top + init_info->height : info.client_area.bottom;
        info.style |= parent ? WS_CHILD : WS_OVERLAPPEDWINDOW;
        RECT rect{ info.client_area };

        // デバイスのサイズに合わせてwindowサイズを調整
        AdjustWindowRect(&rect, info.style, FALSE);

        const wchar_t* caption{ (init_info && init_info->caption) ? init_info->caption : L"Game Graphics" };
        const s32 left{ init_info ? init_info->left : info.top_left.x };
        const s32 top{ init_info ? init_info->top : info.top_left.y };
        const s32 width{ rect.right - rect.left };
        const s32 height{ rect.bottom - rect.top };

        // Windowのインスタンス作成
        info.hwnd = CreateWindowEx(
            0,                // 拡張windowスタイル(ドラッグアンドドロップなどの機能を追加できる項目)
            wc.lpszClassName, // クラス名
            caption,          // タイトルバーに表示される文字列
            info.style,       // スタイル
            left, top,        // 初期位置
            width, height,    // 初期サイズ
            parent,           // 親ウィンドウのハンドル
            NULL,             // ハンドルメニュー
            NULL,             // インスタンスハンドル
            NULL              // 追加作成パラメータ
        );

        if (info.hwnd)
        {
            //エラーの誤報を防ぐために一度クリア
            DEBUG_OP(SetLastError(0));
            const window_id id{ AddToWindows(info) };
            //USERDATA領域にIDを設定
            SetWindowLongPtr(info.hwnd, GWLP_USERDATA, (LONG_PTR)id);
            //追加バイト領域にコールバック関数を設定
            if (callback) SetWindowLongPtr(info.hwnd, 0, (LONG_PTR)callback);
            assert(GetLastError() == 0);
            ShowWindow(info.hwnd, SW_SHOWNORMAL);
            UpdateWindow(info.hwnd);
            return Window{ id };
        }
        return {};
    }

    void RemoveWindow(window_id id)
    {
        WindowInfo& info{ GetFromId(id) };
        DestroyWindow(info.hwnd);
        RemoveFromWindows(id);
    }

}
