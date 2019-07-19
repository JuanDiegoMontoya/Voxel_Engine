#pragma once

namespace Debug 
{
	extern std::map<std::string, double> systemsPerfInfo;

	void Update(float dt);
}

//#ifdef _DEBUG
#if 1
#define PERF_BENCHMARK_START  high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now(); 
#define PERF_BENCHMARK_END \
	duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_); \
	std::pair<std::string, double> benchmark_data_; \
	benchmark_data_.first = __FUNCTION__; \
	benchmark_data_.second = benchmark_duration_.count() * 1000; \
	Debug::systemsPerfInfo[benchmark_data_.first] += benchmark_data_.second; 
#else
//noops
#define PERF_BENCHMARK_START do { (void)sizeof(int); } while(0);
#define PERF_BENCHMARK_END do { (void)sizeof(int); } while(0);
#endif