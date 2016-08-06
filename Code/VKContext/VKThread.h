// VKThread.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKThread_h__
#define __VKThread_h__


namespace VK {
namespace Thread {

//#define ANDROID
#define VK_MULTI_THREADED
#ifdef VK_MULTI_THREADED

#ifdef _WIN32
inline unsigned int GetCurrentID() { return ::GetCurrentThreadId(); }

class Lock
{
private:
	CRITICAL_SECTION m_sec;

public:
	Lock()	{ ::InitializeCriticalSection(&m_sec); }
	~Lock()	{ ::DeleteCriticalSection(&m_sec); }
	// If I don't make these const, I can't use CCriticalSection in const getter methods!
	void lock() const	{ ::EnterCriticalSection((LPCRITICAL_SECTION)&m_sec); }
	void unlock() const	{ ::LeaveCriticalSection((LPCRITICAL_SECTION)&m_sec); }
};

class Event
{
private:
	HANDLE m_hEvent;

public:
	enum {TimedOut=0, Signalled=1};

	Event()				{ m_hEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL); }
	~Event()			{ ::CloseHandle(m_hEvent); }
	bool set()			{ return ::SetEvent(m_hEvent) ? true : false; }
	bool reset()		{ return ::ResetEvent(m_hEvent) ? true : false; }
	int wait(int nMilliseconds=-1) {
		DWORD dw = (nMilliseconds < 0) ? INFINITE : nMilliseconds;
		dw = ::WaitForSingleObject(m_hEvent, dw);
		if(dw == WAIT_TIMEOUT)
			return TimedOut;
		return Signalled;
	}
};

#endif // _WIN32

#ifdef ANDROID

#include <pthread.h>

class Lock
{
private:
	pthread_mutex_t m_mutex;

public:
	Lock()	{
		pthread_mutexattr_t attr;
		pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		int ii_p = pthread_mutex_init(&m_mutex, &attr);
		pthread_mutexattr_destroy(&attr);
	}
	~Lock()	{
		pthread_mutex_destroy(&m_mutex);
	}
	
	// If I don't make these const, I can't use CCriticalSection in const getter methods!
	void lock() const	{ pthread_mutex_lock((pthread_mutex_t *)&m_mutex); }
	void unlock() const	{ pthread_mutex_unlock((pthread_mutex_t *)&m_mutex); }
};

class Event
{
private:
	pthread_cond_t m_cond;

public:
	enum {TimedOut=0, Signalled=1};

	Event()				{ pthread_cond_init(&m_cond, NULL); }
	~Event()			{ pthread_cond_destroy(&m_cond); }
	bool set()			{ return true; }//pthread_cond_signal(&m_cond); }
	int wait(int nMilliseconds=-1) {
		//pthread_cond_wait(&t.cond, &t.mn);
		//pthread_cond_timedwait(&t.cond, &t.mn, &ts);
		
		//DWORD dw = (nMilliseconds < 0) ? INFINITE : nMilliseconds;
		//dw = ::WaitForSingleObject(m_hEvent, dw);
		//if(dw == WAIT_TIMEOUT)
		//	return TimedOut;
		return Signalled;
	}
};

#endif // ANDROID

class AutoLock
{
protected:
	const Lock &m_lock;
	bool m_bLocked;

public:
	AutoLock(const Lock &lock) : m_lock(lock) {
		m_lock.lock();
		m_bLocked = true;
	}
	~AutoLock() {
		if(m_bLocked)
			m_lock.unlock();
	}
	void unlock() {
		m_lock.unlock();
		m_bLocked = false;
	}
	void lock() {
		m_lock.lock();
		m_bLocked = true;
	}
};

#else // VK_MULTI_THREADED

inline unsigned int GetCurrentID() { return 0; }

class Lock
{
public:
	void lock() const	{}
	void unlock() const	{}
};

class Event
{
public:
	enum {TimedOut=0, Signalled=1};
	bool set()			{ return true; }
	bool reset()		{ return true; }
	int wait(int nMilliseconds=-1) { return Signalled; }
};

class AutoLock
{
public:
	AutoLock(const Lock &lock) {}
	void unlock() {}
	void lock() {}
};

#endif // VK_MULTI_THREADED

} // namespace Thread
} // namespace VK

#endif // __VKThread_h__
