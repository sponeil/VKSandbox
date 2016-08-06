// VKTimer.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKTimer_h__
#define __VKTimer_h__

#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

namespace VK {

#define DATETIME24_FORMAT "%Y-%m-%d %H:%M:%S"

/// Encapsulates a high-precision timer for tracking time offsets.
/// It is MUCH more precise than something like ::GetTickCount().
/// It is a singleton because one instance will suffice for an entire app.
/// (It should be thread-safe because all methods are const after constructor.)
class Timer
{
protected:
#ifdef _WIN32
	static double m_dFactor;   /// Multiplier for the performance counter to convert to seconds
	static uint64_t m_nStart;  /// The value of the precision timer on initialization
	static double m_dEpoch;    /// The epoch time on initialization
#endif
    
public:
#ifdef _WIN32
	static void Init();             /// Initializes the high-precision timer (Windows-only)
#endif
	static double Time();           /// Returns a high-res time value (number of seconds and fractions of a second since epoch)
	static uint32_t Tick();         /// Returns a low-res system up-time (number of ms since OS started, wraps every 49 days)
	static void Sleep(uint32_t ms); /// Sleeps for a number of ms (low-res for Windows)

	static char *Local(char *pszDest, double dTime, bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		time_t t = (time_t)dTime;
		dTime -= t; // To avoid overflow in the multiply below
		uint32_t n = (uint32_t)strftime(pszDest, 32, pszFormat, localtime(&t));
		if(ms) sprintf(pszDest+n, ".%3.3u", (uint32_t)(dTime * 1000.0) % 1000);
		return pszDest;
	}

	static char *GMT(char *pszDest, double dTime, bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		time_t t = (time_t)dTime;
		dTime -= t; // To avoid overflow in the multiply below
		uint32_t n = (uint32_t)strftime(pszDest, 32, pszFormat, gmtime(&t));
		if(ms) sprintf(pszDest+n, ".%3.3u", (uint32_t)(dTime * 1000.0) % 1000);
		return pszDest;
	}

	static char *Local(char *pszDest, bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		return Local(pszDest, Time(), ms, pszFormat);
	}
	static char *GMT(char *pszDest, bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		return GMT(pszDest, Time(), ms, pszFormat);
	}
	static std::string Local(bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		char szDest[32];
		return Local(szDest, Time(), ms, pszFormat);
	}
	static std::string GMT(bool ms=false, const char *pszFormat=DATETIME24_FORMAT) {
		char szDest[32];
		return GMT(szDest, Time(), ms, pszFormat);
	}
};

} // namespace VK

#endif // __VKTimer_h__
