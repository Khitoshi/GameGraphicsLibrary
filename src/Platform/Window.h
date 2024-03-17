#pragma once

#include "CommonHeaders.h"
#include "PlatformTypes.h"

#include <Windows.h>

namespace Platform {

    DEFINE_TYPED_ID(window_id);
    class Window
    {
    public:
        constexpr explicit Window(window_id id) : _id{ id } {}
        constexpr Window() : _id{ Id::invalid_id } {}
        constexpr window_id GetId() const { return _id; }
        constexpr bool IsValid() const { return Id::IsValid(_id); }

        void SetFullscreen(bool is_fullscreen) const;
        bool IsFullscreen() const;
        void* Handle() const;
        void SetCaption(const wchar_t* caption) const;
        Math::u32v4 Size() const;
        void Resize(u32 width, u32 height) const;
        u32 Width() const;
        u32 Height() const;
        bool IsClosed() const;
    private:
        window_id _id{ Id::invalid_id };
    };

    struct WindowInitInfo;
    Window GenerateWindow(const WindowInitInfo* const init_info = nullptr);
    void RemoveWindow(window_id id);
}