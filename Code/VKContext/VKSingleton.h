// VKSingleton.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKUtil_h__
#define __VKUtil_h__

namespace VK {

class NoCopy {
private:
	NoCopy(const NoCopy &copy) { throw "You can't copy this!"; }
protected:
	NoCopy() {}
};

/// A templatized base class for making a class into a singleton.
/// The singleton can be created and destroyed dynamically, but there can
/// only be one. Static methods will return a reference to the singleton.
template <class T>
class Singleton
{
private:
	static T *m_pSingleton; ///< The global pointer to the singleton

protected:
	/// Default constructor, throws an exception if singleton already exists
	Singleton() {
		if(m_pSingleton != NULL && VK::Throw)
			VK::Throw("Singleton already exists");
		m_pSingleton = (T *)this;	// Not sure about other compilers, but MSVC++ is smart enough to cast this correctly

		// Manual way to do the pointer arithmetic
		//int nOffset = (int)(T*)1 - (int)(Singleton <T>*)(T*)1;
		//m_pSingleton = (T*)((int)this + nOffset);
	}
	/// Default destructor, throws an exception if singleton does not exist
	~Singleton() {
		if(m_pSingleton == NULL && VK::Throw)
			VK::Throw("Singleton does not exist");
		m_pSingleton = NULL;
	}

public:
	/// Call to see if the singleton is valid.
	static bool IsValid()	{ return m_pSingleton != NULL; }

	/// Call to get a reference to the singleton.
	/// Throws an exception if the singleton does not exist, so you must
	/// call IsValid() first if you are not certain.
	static T &GetRef() {
		if(m_pSingleton == NULL && VK::Throw)
			VK::Throw("Singleton does not exist");
		return *m_pSingleton;
	}
};

// Because it's templatized, this static member doesn't need to be declared in a CPP file
template <class T> T *Singleton<T>::m_pSingleton = NULL;

}// namespace VK

#endif // __VKUtil_h__
