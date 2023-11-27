#pragma once
#include <thread>

#define TEST_ENTITY_COMPONENTS 0
#define TEST_WINDOW 0
#define TEST_RENDERER 1

class Test
{
public:
	virtual bool Initialize() = 0;
	virtual void Run() = 0;
	virtual void Shutdown() = 0;

};

#if _WIN64
#include <Windows.h>
class Timer
{
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimeStamp = std::chrono::steady_clock::time_point;

    // ïΩãœÉtÉåÅ[ÉÄéûä‘(ïb)
    constexpr float AverageFrameSecond() const { return _dt_avg * 1e-6f; }

    void Begin()
    {
        _start = Clock::now();
    }

    void End()
    {
        auto dt = Clock::now() - _start;
        _us_avg += ((float)std::chrono::duration_cast<std::chrono::microseconds>(dt).count() - _us_avg) / (float)_counter;
        ++_counter;
        _dt_avg = _us_avg;

        if (std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - _seconds).count() >= 1)
        {
            OutputDebugStringA("Avg. frame (ms): ");
            OutputDebugStringA(std::to_string(_us_avg * 0.001f).c_str());
            OutputDebugStringA((" " + std::to_string(_counter)).c_str());
            OutputDebugStringA(" fps");
            OutputDebugStringA("\n");
            _us_avg = 0.f;
            _counter = 1;
            _seconds = Clock::now();
        }
    }

private:
    float       _dt_avg{ 16.7f };
    float       _us_avg{ 0.f };
    int         _counter{ 1 };
    TimeStamp  _start;
    TimeStamp  _seconds{ Clock::now() };
};
#endif // _WIN64
