#pragma once
#include "CommonHeaders.h"

namespace Rizityo::Time
{
    // TODO : ÉtÉåÅ[ÉÄïΩãœ
    class Timer
    {
    public:
        
        Timer() = default;
        DISABLE_COPY_AND_MOVE(Timer);

        void Reset();
        void Start();
        void Stop();
        void Tick();

        [[nodiscard]] constexpr float32 DeltaTime() const { return _DeltaTime * 1e-6f; }
        [[nodiscard]] constexpr float32 TotalTime() const;
        [[nodiscard]] constexpr bool IsStopped() const { return _Stopped; }

    private:

        using Clock = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::steady_clock::time_point;
        using MicroSeconds = std::chrono::duration<float32, std::micro>;

        TimePoint _StartTime;
        TimePoint _StopTime;
        TimePoint _CurrentTime;
        TimePoint _PrevTime;
        MicroSeconds _PausedTime;

        float32 _DeltaTime; // (ïb)
        bool _Stopped = false;
    };
}


