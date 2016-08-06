// VKProfiler.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKProfiler_h__
#define __VKProfiler_h__

namespace VK {

namespace Profile {
/// Encapsulates a profiler timer.
class Timer
{
protected:
	Timer *m_pParent;
	std::map<std::string, Timer*> m_mapChildren;

	std::string m_strName;
	double m_dStartTime;
	double m_dTotalTime;
	unsigned long m_nCallCount;

public:
	Timer(std::string strName, Timer *pParent);
	~Timer();
	const char *getName() const { return m_strName.c_str(); }
	std::string getProfilerStats(int nLevel=0);
	void reset();
	Timer *getTimer(const char *pszName);
	void startTimer();
	void stopTimer();
};

/// Encapsulates a profiler sample.
class Sample
{
protected:
	Sample *m_pParentSample;
	Timer *m_pTimer;

public:
	Sample(std::string strName, int nProfilingLevel);
	~Sample();
};
} // namespace Profile

/// Encapsulates a simple code profiler.
class Profiler : public Singleton<Profiler>
{
private:
	friend class Profile::Sample;

	Thread::Lock m_lock;
	std::string m_strName;
	std::map<unsigned int, Profile::Sample *> m_mapCurrentSample;
	std::map<unsigned int, Profile::Timer *> m_mapRootTimer;
	int m_nProfilingLevel;

	Profile::Timer *getRootTimer();
	Profile::Sample *getCurrentSample();
	Profile::Sample *setCurrentSample(Profile::Sample *p);

public:
	Profiler(std::string strName, int nProfilingLevel);
	~Profiler();
	bool isProfiled(int nLevel) const { return (m_nProfilingLevel != 0 && nLevel <= m_nProfilingLevel); }
};

} // namespace VK

#define ENABLE_PROFILING
#ifdef ENABLE_PROFILING
#define VKPROFILE(name, level) VK::Profile::Sample _sample(name, level);
#else
#define VKPROFILE(name, level)
#endif

#endif // __VKProfiler_h__
