#pragma once
#include <chrono>

#if _WIN64
#include <Windows.h>
class Time
{
public:
    using clock = std::chrono::high_resolution_clock;
    using time_stamp = std::chrono::steady_clock::time_point;

    void Begin()
    {
        start_ = clock::now();
    }

    void End()
    {
        auto dt = clock::now() - start_;
        ms_avg_ += ((float)std::chrono::duration_cast<std::chrono::milliseconds>(dt).count() - ms_avg_) / (float)counter_;
        ++counter_;

        if (std::chrono::duration_cast<std::chrono::seconds>(clock::now() - seconds_).count() >= 1)
        {
            OutputDebugStringA("Avg. frame (ms): ");
            OutputDebugStringA(std::to_string(ms_avg_).c_str());
            OutputDebugStringA((" " + std::to_string(counter_)).c_str());
            OutputDebugStringA(" fps");
            OutputDebugStringA("\n");
            ms_avg_ = 0.f;
            counter_ = 1;
            seconds_ = clock::now();
        }
    }

private:
    float       ms_avg_{ 0.f };
    int         counter_{ 1 };
    time_stamp  start_;
    time_stamp  seconds_{ clock::now() };
};
#endif // _WIN64