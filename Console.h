#pragma once

class Console
{
private:
	Console();
	~Console();
public:
	inline static Console* instance_ = nullptr;
	static Console* GetInstance() { return instance_; }
	static bool Alloc();
	static bool Free();
};
