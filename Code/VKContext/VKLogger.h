// VKLogger.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKLogger_h__
#define __VKLogger_h__

namespace VK {

//#define THROW_EXCEPTION // Comment out if you don't want logException to throw the message
const int LOGGER_BUFFER_SIZE = 16384;

/// A simple but very efficient and powerful logging class.
/// Its primary design goal is to have zero overhead for logging levels that
/// are not enabled. This allows you to put logging calls almost anywhere
/// in your code without having to worry about the performance implications.
/// Aside from initializing it, its members will rarely be called directly.
/// The macros below make it much easier and safer to use efficiently.
/// They ensure that none of your arguments are evaluated if the specified
/// logging level is not enabled, and they automatically pass the __FILE__
/// and __LINE__ macros for you.
class Logger : public Singleton<Logger>
{
protected:
#ifndef ANDROID
	Thread::Lock m_lock;
	const char **m_pszSeverity;
	std::ofstream m_ofLog;
#endif
	
	std::string m_strLogName;
	int m_nLoggingLevel;
	char m_szException[LOGGER_BUFFER_SIZE];

public:
	/// Valid logging levels
	enum Level {
		DefaultLevel = -1, ///< Only used to tell the Logger constructor to default the logging level
		None = 0,		///< Use to disable logging entirely
		Critical = 1,	///< Use to log critical errors
		Error = 2,	 	///< Use to log non-critical errors
		Warning = 3,	///< Use to log warnings
		Info = 4,		///< Use to log key informational events
		Notice = 5,		///< Use to log more common notices
		Debug = 6,		///< Use to log debug events
		Spam = 7,		///< Use to log anything that can severely bloat the log files
		Levels = 8		///< The total number of logging levels
	};

	/// Constructs the singleton logger.
	/// @param[in] pszLogName The name of the file to log to
	/// @param[in] nLevel The logging level to use
	Logger(const char *pszLogName = "VKContext.log", Level nLevel=DefaultLevel);

	/// Call to check to see if the specified severity is currently being logged.
	bool isLogged(Level nSeverity) const { return (nSeverity <= m_nLoggingLevel); }

	/// Call to log a fixed message of any size.
	bool logMessage(const char *pszFile, int nLine, Level nSeverity, const char *pszMessage);

	/// Call to log an sprintf-style formatted message (up to LOGGER_BUFFER_SIZE characters)
	bool logFormattedMessage(const char *pszFile, int nLine, Level nSeverity, const char *pszFormat, ...);

	/// Call to log an sprintf-style formatted message (up to LOGGER_BUFFER_SIZE characters)
	bool logFormattedMessage_va(const char *pszFile, int nLine, Level nSeverity, const char *pszFormat, va_list args);

	/// Call to log a critical message and raise an exception (if THROW_EXCEPTION is defined)
	bool logException(const char *pszFile, int nLine, const char *pszFormat, ...);

	/// A convenient method for formatting a message and returning a string
	static std::string format(const char *pszFormat, ...);
};

// In addition to making logging calls simple, these macros are critical to performance.
// If the variable args call expensive functions, we do NOT want to call them when that logging level is disabled.
// The macros return true iff the message was logged, though that was done primarily to let me use the ternary operator
// and put parentheses around the macro (which helps avoid compilation problems when the macro is expanded).
#ifdef _WIN32
#define VKLogCritical(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Critical) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Critical, format, __VA_ARGS__) : false)
#define VKLogError(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Error) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Error, format, __VA_ARGS__) : false)
#define VKLogWarning(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Warning) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Warning, format, __VA_ARGS__) : false)
#define VKLogInfo(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Info) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Info, format, __VA_ARGS__) : false)
#define VKLogNotice(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Notice) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Notice, format, __VA_ARGS__) : false)
#define VKLogDebug(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Debug) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Debug, format, __VA_ARGS__) : false)
#define VKLogSpam(format, ...) (VK::Logger::GetRef().isLogged(VK::Logger::Spam) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Spam, format, __VA_ARGS__) : false)
#define VKLogException(format, ...) (VK::Logger::GetRef().logException(__FILE__, __LINE__, format, __VA_ARGS__))
#else
#define VKLogCritical(...) (VK::Logger::GetRef().isLogged(VK::Logger::Critical) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Critical, __VA_ARGS__) : false)
#define VKLogError(...) (VK::Logger::GetRef().isLogged(VK::Logger::Error) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Error, __VA_ARGS__) : false)
#define VKLogWarning(...) (VK::Logger::GetRef().isLogged(VK::Logger::Warning) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Warning, __VA_ARGS__) : false)
#define VKLogInfo(...) (VK::Logger::GetRef().isLogged(VK::Logger::Info) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Info, __VA_ARGS__) : false)
#define VKLogNotice(...) (VK::Logger::GetRef().isLogged(VK::Logger::Notice) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Notice, __VA_ARGS__) : false)
#define VKLogDebug(...) (VK::Logger::GetRef().isLogged(VK::Logger::Debug) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Debug, __VA_ARGS__) : false)
#define VKLogSpam(...) (VK::Logger::GetRef().isLogged(VK::Logger::Spam) ? VK::Logger::GetRef().logFormattedMessage(__FILE__, __LINE__, VK::Logger::Spam, __VA_ARGS__) : false)
#define VKLogException(...) (VK::Logger::GetRef().logException(__FILE__, __LINE__, __VA_ARGS__))
#endif

#ifndef ANDROID

/// Logs entry and exit points of the current scope (via the VKLogScope macro).
/// VKLogScope is extremely powerful if you have exception handling enabled
/// because even though it normally does nothing when you have the Spam level
/// disabled, it will still log an error with the file and line# if the scope
/// is unwound by an exception. This can show you the entire call stack when
/// an exception occurs in Release mode, and it incurs practically no overhead
/// when no exceptions are thrown.
template <class SCOPE, Logger::Level LEVEL>
class TScopeLog
{
protected:
	double m_dStart; ///< The time we entered this scope (only used if LEVEL is enabled)
	char *m_pszMessage; ///< The message to log on scope entry and exit (only used if LEVEL is enabled)

// I wanted the line number to be a template argument, but __LINE__ is not constant when using "Edit and Continue"
// So I use a member variable in Debug mode and a static method of the SCOPE struct in Release mode
#ifdef _DEBUG
	int m_nLine;
	int getLine() { return m_nLine; }
#else
	int getLine() { return SCOPE::LINE(); }
#endif
	const char *getFile() { return SCOPE::FILE(); }
	const char *getFunc() { return SCOPE::FUNC(); }

	void Init(const char *pszFormat, va_list &args) {
		if(VK::Logger::GetRef().isLogged(LEVEL)) {
			// Normally I would avoid using new/delete here, but since LEVEL is almost always Spam,
			// and the Spam logging level is almost never enabled, this code is almost never used,
			// and std::string would have a much heavier footprint here when it's not being used.
			// The most important design goal of this logging code is that it cost nothing when logging is disabled.
			char szBuffer[1024];
			int n = vsnprintf(szBuffer, 1023, pszFormat, args);
			szBuffer[1023] = 0;
			m_pszMessage = new char[n+1];
			memcpy(m_pszMessage, szBuffer, n+1);
			VK::Logger::GetRef().logFormattedMessage(getFile(), getLine(), LEVEL, "Entering %s", m_pszMessage);
			m_dStart = VK::Timer::Time();
		}
	}

public:
#ifdef _DEBUG
	TScopeLog(int nLine, const char *pszFormat, ...) : m_dStart(0), m_pszMessage(NULL), m_nLine(nLine) {
#else
	TScopeLog(const char *pszFormat, ...) : m_dStart(0), m_pszMessage(NULL) {
#endif
		if(VK::Logger::GetRef().isLogged(LEVEL)) {
			va_list args;
			va_start(args, pszFormat);
			Init(pszFormat, args);
			va_end(args);
		}
	}

	~TScopeLog() {
		float fTime = m_pszMessage ? (float)(VK::Timer::Time() - m_dStart) : 0.0f;
		if(std::uncaught_exception()) { // If we're leaving the scope by an exception unwind, always log an error
			VK::Logger::GetRef().logFormattedMessage(getFile(), getLine(), VK::Logger::Critical, 
				m_pszMessage ? "Exiting %s by exception (%.4f seconds)" : "Exiting %s by exception",
				m_pszMessage ? m_pszMessage : getFunc(), fTime);
		} else if(m_pszMessage) // Otherwise only log if the scope was entered at the proper logging level
			VK::Logger::GetRef().logFormattedMessage(getFile(), getLine(), LEVEL, "Exiting %s (%.4f seconds)", m_pszMessage, fTime);
		delete m_pszMessage;
	}
};


// Creating a local scope struct is ugly, but it's needed to pass constant strings like __FILE__ via template arg.
// With "Edit and Continue", __LINE__ is not constant, so it can't be passed in via template arg.
// As a result, I make it a constructor parameter of TScopeLog in Debug mode.
#ifdef _DEBUG
#define VKLogScope(...)\
	struct SCOPE__Struct { \
		static const char *FILE() { return __FILE__; }\
		static const char *FUNC() { return __FUNCTION__; }\
		static int LINE() { return 0; }\
	};\
	VK::TScopeLog<SCOPE__Struct, VK::Logger::Spam> scope_log(__LINE__, __VA_ARGS__)
#else
#define VKLogScope(...)\
	struct SCOPE__Struct {\
		static const char *FILE() { return __FILE__; }\
		static const char *FUNC() { return __FUNCTION__; }\
		static int LINE() { return __LINE__; }\
	};\
	VK::TScopeLog<SCOPE__Struct, VK::Logger::Spam> scope_log(__VA_ARGS__)
#endif

#endif // ANDROID

} // namespace VK

#endif // __VKLogger_h__
