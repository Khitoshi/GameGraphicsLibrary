#pragma once
#include "CommonHeaders.h"

// 進捗を管理するためのクラス
// 任意の進捗コールバック関数を使用して、進捗の現在値と最大値を通知
class Progression
{
public:
    using ProgressCallback = void(*)(s32, s32);

    Progression() = default;
    explicit Progression(ProgressCallback callback)
        : callback_{ callback }
    {}

    DISABLE_COPY(Progression);

    void Callback(u32 value, u32 max_value)
    {
        value_ = value;
        max_value_ = max_value;
        if (callback_) callback_(value, max_value);
    }

    [[nodiscard]] constexpr u32 max_value() const { return max_value_; }
    [[nodiscard]] constexpr u32 value() const { return value_; }
private:
    ProgressCallback    callback_{ nullptr };
    u32                 value_{ 0 };
    u32                 max_value_{ 0 };
};