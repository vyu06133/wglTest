#pragma once
#include "framework.h"
class FPS
{
private:
    std::chrono::high_resolution_clock::time_point lastTime;
    double elapsedTime = 0.0f;
    int frameCount = 0;
    double fps;

public:
    FPS()
    {
		Reset();
    }
    
	void Reset()
    {
        lastTime = std::chrono::high_resolution_clock::now();
        elapsedTime = 0.0f;
        frameCount = 0;
		fps = 0.0f;
    }

    void Update()
    {
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> delta = now - lastTime;
        lastTime = now;

        elapsedTime += delta.count();
        frameCount++;

        if (elapsedTime >= 1.0)
        {
            fps = frameCount / elapsedTime;
            frameCount = 0;
            elapsedTime = 0.0f;
        }
    }

	double GetFps() const
	{
        return fps;
	}
};

