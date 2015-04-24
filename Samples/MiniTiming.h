#include <chrono>
#include <string>

class MiniTiming
{
public:
	MiniTiming()
	{
		Reset();
	}

	inline void Reset()
	{
		_lastT = std::chrono::high_resolution_clock::now();
	}

	inline float Check(bool reset = false)
	{
		auto dt = std::chrono::high_resolution_clock::now() - _lastT;
		if (reset) Reset();

		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(dt).count();
		return duration / 1000.0f;
		//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(dt).count();
		//return (float)duration;
	}

private:
	std::chrono::time_point<std::chrono::high_resolution_clock> _lastT;
};

class AutoTiming
{
public:
	AutoTiming(std::string msg = ""): _msg(msg){}

	~AutoTiming()
	{
		printf("[Timing]%s:%8.3fms\n", _msg.c_str(), _ti.Check());
	}

private:
	MiniTiming  _ti;
	std::string _msg;
};

#if EnableTiming==1
#define BeginTiming(info) {AutoTiming at_(info);
#define EndGPUTiming cudaDeviceSynchronize();}
#define EndTiming }
#else
#define BeginTiming(info)
#define EndGPUTiming
#define EndTiming
#endif