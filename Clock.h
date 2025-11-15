#pragma once
#include <chrono>
class Clock
{
private:
	std::chrono::system_clock::time_point startTime_;
	std::chrono::system_clock::time_point now_;
public:
	Clock() : startTime_(std::chrono::system_clock::now()), now_(std::chrono::system_clock::now())
	{
	}
	inline float GetDelta()
	{
		now_ = std::chrono::system_clock::now();
		auto dur = now_ - startTime_;
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		return static_cast<float>(msec) * 0.0001f;
	}
	static inline float Now()
	{
		auto now = std::chrono::steady_clock::now();
		auto dur = now.time_since_epoch();
		auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
		return static_cast<float>(msec) * 0.0001f;
	}
	inline void Period()
	{
		startTime_ = now_;
	}
};

