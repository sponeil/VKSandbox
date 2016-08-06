// VKLogger.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"

#ifdef ANDROID
#include <android/log.h>
#endif

namespace VK {

Logger::Logger(const char *pszLogName, Level nLevel) {
	m_strLogName = pszLogName;
	m_nLoggingLevel = nLevel;
	if(nLevel == DefaultLevel) {
#ifdef _DEBUG
		m_nLoggingLevel = Debug;
#else
		m_nLoggingLevel = Notice;
#endif
	}

#ifndef ANDROID
	static const char *pszSeverity[] = {"", "CRITICAL", "ERROR", "WARNING", "INFO", "NOTICE", "DEBUG", "SPAM"};
	m_pszSeverity = pszSeverity;
	Path::Log().mkdir();
	m_ofLog.open(Path::Log() + pszLogName);
#endif
}

bool Logger::logMessage(const char *pszFile, int nLine, Level nSeverity, const char *pszMessage) {
	char szTime[32];
	Timer::Local(szTime, true);
#ifdef ANDROID
	char szHeader[1024];
	sprintf(szHeader, "%s:%d:%s", pszFile, nLine, szTime);
	__android_log_print(
		nSeverity == Critical ? ANDROID_LOG_ERROR :
		nSeverity == Error ? ANDROID_LOG_ERROR :
		nSeverity == Warning ? ANDROID_LOG_WARN :
		nSeverity == Info ? ANDROID_LOG_INFO :
		nSeverity == Notice ? ANDROID_LOG_INFO :
		nSeverity == Debug ? ANDROID_LOG_DEBUG :
		ANDROID_LOG_VERBOSE, szHeader, pszMessage);
#else
	if(!m_ofLog.is_open())
		return false;

	Thread::AutoLock lock(m_lock);
	m_ofLog << "Time: " << szTime << " " << "Severity: " << m_pszSeverity[nSeverity];
	if(pszFile && *pszFile) {
		const char *p = strrchr(pszFile, '/');
		if(p++ == NULL) p = strrchr(pszFile, '\\');
		if(p++ == NULL) p = pszFile;
		m_ofLog << " Location: " << p << ":" << nLine;
	}
	m_ofLog << std::endl << pszMessage << std::endl << std::endl;
	m_ofLog.flush(); // Not ideal for performance, but needed in case we crash
#endif
	return true;
}

std::string Logger::format(const char *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	char szBuffer[LOGGER_BUFFER_SIZE];
	vsnprintf(szBuffer, LOGGER_BUFFER_SIZE-1, pszFormat, va);
	szBuffer[LOGGER_BUFFER_SIZE-1] = 0;
	va_end(va);
	return szBuffer;
}

bool Logger::logFormattedMessage(const char *pszFile, int nLine, Level nSeverity, const char *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	logFormattedMessage_va(pszFile, nLine, nSeverity, pszFormat, va);
	va_end(va);
	return true;
}

bool Logger::logFormattedMessage_va(const char *pszFile, int nLine, Level nSeverity, const char *pszFormat, va_list args)
{
	char szBuffer[LOGGER_BUFFER_SIZE];
	vsnprintf(szBuffer, LOGGER_BUFFER_SIZE-1, pszFormat, args);
	szBuffer[LOGGER_BUFFER_SIZE-1] = 0;
	logMessage(pszFile, nLine, nSeverity, szBuffer);
	return true;
}

bool Logger::logException(const char *pszFile, int nLine, const char *pszFormat, ...)
{
	va_list va;
	va_start(va, pszFormat);
	if(pszFormat == NULL)
		pszFormat = "NULL exception";
	vsnprintf(m_szException, LOGGER_BUFFER_SIZE-1, pszFormat, va);
	m_szException[LOGGER_BUFFER_SIZE-1] = 0;
	bool bLog = logMessage(pszFile, nLine, Critical, m_szException);
	va_end(va);
	if(VK::Throw)
		VK::Throw(m_szException);
	return bLog;
}

} // namespace VK
