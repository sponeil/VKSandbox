// VKTimer.cpp
//

#include "VKCore.h"

namespace VK {

#ifdef _WIN32
double Timer::m_dFactor;   /// Multiplier for the performance counter to convert to seconds
uint64_t Timer::m_nStart;  /// The value of the precision timer on initialization
double Timer::m_dEpoch;    /// The epoch time on initialization

void Timer::Init() {
	LARGE_INTEGER nFrequency;
	::QueryPerformanceFrequency(&nFrequency);
	m_dFactor = 1.0 / nFrequency.QuadPart;
	LARGE_INTEGER nNow;
	::QueryPerformanceCounter(&nNow);
	m_nStart = (uint64_t)nNow.QuadPart;

	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t tt = ((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime;
	tt -= 116444736000000000ULL;
	m_dEpoch = tt / 10000000.0;
}
#endif

double Timer::Time() {
#ifdef _WIN32
	LARGE_INTEGER nNow;
	::QueryPerformanceCounter(&nNow);
	return m_dEpoch + ((uint64_t)nNow.QuadPart - m_nStart) * m_dFactor;
#else
	timeval tNow;
	::gettimeofday(&tNow, NULL);
	return tNow.tv_sec + tNow.tv_usec / 1000000.0;
#endif
}

uint32_t Timer::Tick() {
	uint32_t t;
#ifdef _WIN32
	t = ::GetTickCount();
#else
	struct timespec now;
	if( clock_gettime( CLOCK_MONOTONIC, &now ) )
		return 1;
	t = now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
#endif
	if(t == 0) // 0 is considered an "invalid" time, so offset it by 1ms
		t = 1;
	return t;
}

void Timer::Sleep(uint32_t ms) {
#ifdef _WIN32
	::Sleep(ms);
#else
	usleep(ms * 1000);
#endif
}

} // namespace VK
