// VKProfiler.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"

namespace VK {
namespace Profile {

Timer::Timer(std::string strName, Timer *pParent)
{
	m_strName = strName;
	m_pParent = pParent;
	m_dStartTime = -1;
	m_dTotalTime = 0;
	m_nCallCount = 0;
}

Timer::~Timer()
{
	// Clean up the child timers to avoid memory leaks
	std::map<std::string, Timer*>::iterator i;
	for(i = m_mapChildren.begin(); i != m_mapChildren.end(); i++)
	{
		Timer *pChild = i->second;
		delete pChild;
	}
}

std::string Timer::getProfilerStats(int nLevel)
{
	static char szBuffer[1024];
	sprintf(szBuffer, "%8u : %8u : %5.1f : %*.*s%s (%.3f ms per call)\n", m_nCallCount,
		(unsigned int)(m_dTotalTime * 1000.0 + 0.5),
		m_pParent ? (float)(m_dTotalTime*100.0 / m_pParent->m_dTotalTime) : 100.0f,
		nLevel, nLevel, " ", m_strName.c_str(), (float)(1000.0*m_dTotalTime/m_nCallCount));

	std::string strBuild = szBuffer;
	if(nLevel == 0)
		strBuild = "   Count : Time(ms) :     % : Profile Name\n-------------------------------------------------------------\n" + strBuild;
	for(std::map<std::string, Timer*>::iterator i = m_mapChildren.begin(); i != m_mapChildren.end(); i++)
		strBuild += i->second->getProfilerStats(nLevel+1);
	return strBuild;
}

void Timer::reset()
{
	m_dTotalTime = 0;
	m_nCallCount = 0;
}

Timer *Timer::getTimer(const char *pszName)
{
	Timer *&pTimer = m_mapChildren[pszName];
	if(pTimer == NULL)
		pTimer = new Timer(pszName, this);
	return pTimer;
}

void Timer::startTimer()
{
	if(m_dStartTime > 0) {
		VKLogException("Trying to start a profile timer that's already started!");
		return;
	}
	m_dStartTime = VK::Timer::Time();
}

void Timer::stopTimer()
{
	if(m_dStartTime < 0) {
		VKLogException("Trying to stop a profile timer that was never started!");
		return;
	}
	double dEndTime = VK::Timer::Time();
	m_dTotalTime += dEndTime - m_dStartTime;
	m_dStartTime = -1;
	m_nCallCount++;
}


Sample::Sample(std::string strName, int nProfilingLevel)
{
	m_pTimer = NULL;
	if(VK::Profiler::GetRef().isProfiled(nProfilingLevel)) {
		Sample *pCurrentSample = VK::Profiler::GetRef().getCurrentSample();
		m_pParentSample = pCurrentSample;
		VK::Profiler::GetRef().setCurrentSample(this);
		m_pTimer = (m_pParentSample ? m_pParentSample->m_pTimer : VK::Profiler::GetRef().getRootTimer())->getTimer(strName.c_str());
		m_pTimer->startTimer();
	}
}

Sample::~Sample()
{
	if(m_pTimer) {
		m_pTimer->stopTimer();
		VK::Profiler::GetRef().setCurrentSample(m_pParentSample);
	}
}

} // namespace Profile

Profiler::Profiler(std::string strName, int nProfilingLevel)
{
	m_strName = strName;
	m_nProfilingLevel = nProfilingLevel;
}

Profiler::~Profiler()
{
	for(std::map<unsigned int, Profile::Timer *>::iterator it=m_mapRootTimer.begin(); it!=m_mapRootTimer.end(); it++) {
		char szBuffer[256];
		sprintf(szBuffer, "Profiling info for thread %d:\n", it->first);
		it->second->stopTimer();
		VK::Logger::GetRef().logMessage(__FILE__, __LINE__, VK::Logger::Debug, (std::string(szBuffer) + it->second->getProfilerStats()).c_str());
		delete it->second;
	}
}

Profile::Timer *Profiler::getRootTimer() {
	Thread::AutoLock lock(m_lock);
	Profile::Timer *p = m_mapRootTimer[Thread::GetCurrentID()];
	if(p == NULL) {
		m_mapRootTimer[Thread::GetCurrentID()] = p = new Profile::Timer(m_strName, NULL);
		p->startTimer();
	}
	return p;
}

Profile::Sample *Profiler::getCurrentSample() {
	Thread::AutoLock lock(m_lock);
	return m_mapCurrentSample[Thread::GetCurrentID()];
}

Profile::Sample *Profiler::setCurrentSample(Profile::Sample *p) {
	Thread::AutoLock lock(m_lock);
	m_mapCurrentSample[Thread::GetCurrentID()] = p;
	return p;
}

} // namespace VK

