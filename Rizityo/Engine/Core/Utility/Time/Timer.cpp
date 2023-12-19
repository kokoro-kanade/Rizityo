#include "Timer.h"

namespace Rizityo::Time
{

	void Timer::Reset()
	{
		_StartTime = Clock::now();
		_PrevTime = _StartTime;
		_StopTime = {};
		_Stopped = false;
	}

	void Timer::Start()
	{
		if (_Stopped)
		{
			_PausedTime += (Clock::now() - _StopTime);
			_PrevTime = Clock::now();
			_StopTime = {};
			_Stopped = false;
		}
	}

	void Timer::Stop()
	{
		if (!_Stopped)
		{
			_StopTime = Clock::now();
			_Stopped = true;
		}
	}

	void Timer::Tick()
	{
		if (_Stopped)
		{
			_DeltaTime = 0.f;
			return;
		}

		_CurrentTime = Clock::now();

		_DeltaTime = static_cast<float32>(std::chrono::duration_cast<std::chrono::microseconds>(_CurrentTime - _PrevTime).count());
		if (_DeltaTime < 0.f)
		{
			_DeltaTime = 0.f;
		}

		_PrevTime = _CurrentTime;

	}

	constexpr float32 Timer::TotalTime() const
	{
		if (_Stopped)
			return (std::chrono::duration_cast<std::chrono::microseconds>(_StopTime - _StartTime) - _PausedTime).count();
		else
			return (std::chrono::duration_cast<std::chrono::microseconds>(_CurrentTime - _StartTime) - _PausedTime).count();
	}
}