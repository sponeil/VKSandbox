// VKDatabase.cpp
//

#include "VKCore.h"
#include "VKDatabase.h"
#include <sqlite3.h>

namespace VK {
namespace DB {

void Connection::handleError(const char *pszMessage, bool bException) {
	int nCode = sqlite3_errcode(m_pConn);
	handleError(nCode, pszMessage, bException);
}

void Connection::handleError(int nCode, const char *pszMessage, bool bException) {
	if(nCode != SQLITE_OK) {
		char szBuffer[16384];
		snprintf(szBuffer, 16383, "%s\nError Code: %d, Extended Code: %d, Error Message: %s", pszMessage, nCode, sqlite3_extended_errcode(m_pConn), sqlite3_errmsg(m_pConn));
		szBuffer[16383] = 0;
		m_strLastError = szBuffer;
		Logger::GetRef().logMessage(__FILE__, __LINE__, Logger::Error, szBuffer);
		if(bException)
			throw m_strLastError.c_str();
	}
}

bool Connection::open(Path path, int nFlags, int nTimeout) {
	Thread::AutoLock lock(m_lock);
	VKLogDebug("Opening database %s", path.c_str());
	if(m_pConn != NULL) {
		sqlite3_close(m_pConn);
		m_pConn = NULL;
	}
	int nResult = sqlite3_open_v2(path, &m_pConn, nFlags | SQLITE_OPEN_NOMUTEX, NULL);
	if(nResult != SQLITE_OK) {
		m_pConn = NULL; // Make sure this wasn't set
		handleError(nResult, "Failed to open database", m_bThrowExceptions);
		return false;
	}
	sqlite3_busy_timeout(m_pConn, nTimeout);

	// If we can't set the pragmas we want (usually due to lock timeout), don't allow the open call to succeed
	// (Have to be careful how we handle error message logging and exceptions here)
	bool bTemp = m_bThrowExceptions;
	m_bThrowExceptions = false; // This will stop exec from throwing an exception
	if(!exec("PRAGMA foreign_keys=1; PRAGMA synchronous = OFF;")) { // But exec will still log the error and save it in m_strLastError
		sqlite3_close(m_pConn);
		m_pConn = NULL;
	}
	m_bThrowExceptions = bTemp;

	// Ok, if the PRAGMA call above failed, decide whether to raise the exception or just return false
	if(m_pConn == NULL) {
		if(m_bThrowExceptions)
			throw m_strLastError.c_str();
		return false;
	}

	m_path = path;
	return true;
}

void Connection::close() {
	Thread::AutoLock lock(m_lock);
	VKLogDebug("Closing database %s", m_path.c_str());
	if(m_pConn != NULL) {
		if(m_bInTransaction) {
			VKLogError("Closing database connection with open transaction");
			try { rollbackTransaction(); } catch(...) { VKLogError("Failed to rollback while closing connection"); }
		}
		try { sqlite3_close(m_pConn); } catch(...) { VKLogError("Failed to close connection, abandoning it"); }
		m_pConn = NULL;
	}
}

bool Connection::backup(Path path) {
	Thread::AutoLock lock(m_lock);
	bool success = false;
	if(path.exists())
		path.del();
	sqlite3 *pDest;
	int nResult = ::sqlite3_open_v2(path, &pDest, Create | ReadWrite | SQLITE_OPEN_NOMUTEX, NULL);
	if(nResult != SQLITE_OK) {
		VKLogWarning("Failed to open backup database %s\nError Code: %d, Extended Code: %d, Error Message: %s", (const char *)path, nResult, sqlite3_extended_errcode(pDest), sqlite3_errmsg(pDest));
	} else {
		sqlite3_backup *pBackup = ::sqlite3_backup_init(pDest, "main", m_pConn, "main");
		if(pBackup) {
			nResult = ::sqlite3_backup_step(pBackup, -1);
			if(nResult == SQLITE_DONE)
				success = true;
			else
				VKLogWarning("Failed to copy data to backup database %s\nError Code: %d, Extended Code: %d, Error Message: %s", (const char *)path, nResult, sqlite3_extended_errcode(pDest), sqlite3_errmsg(pDest));
			::sqlite3_backup_finish(pBackup);
		} else
			VKLogWarning("Failed to initialize backup to %s\nError Code: %d, Extended Code: %d, Error Message: %s", (const char *)path, nResult, sqlite3_extended_errcode(pDest), sqlite3_errmsg(pDest));
		::sqlite3_close(pDest);
	}
	return success;
}

int Connection::static_callback(void *pCallback, int nColumns, char **pValues,char **pNames) {
	return static_cast<Callback *>(pCallback)->handleRow(nColumns, pValues, pNames);
}

bool Connection::exec(const char *psz, Callback *pCallback, bool bLogError) {
	Thread::AutoLock lock(m_lock);
	char *pszError;
	int nResult = sqlite3_exec(m_pConn, psz, pCallback ? static_callback : NULL, static_cast<void *>(pCallback), &pszError);
	if(nResult != SQLITE_OK) {
		std::string strError = pszError; // Need to copy pszError in case handleError() throws an exception
		sqlite3_free(pszError); // Otherwise we won't be able to free it
		if(bLogError)
			handleError(nResult, strError.c_str(), m_bThrowExceptions);
		return false;
	}
	return true;
}

int64_t Connection::insert(const char *psz) {
	Thread::AutoLock lock(m_lock);
	if(!exec(psz))
		return -1;
	return getRowid();
}

int Connection::getInt(const char *psz, int nDefault) {
	Statement stmt(this, psz, m_bThrowExceptions);
	if(stmt.next())
		return stmt.getInt(0, nDefault);
	return nDefault;
}

int64_t Connection::getInt64(const char *psz, int64_t nDefault) {
	Statement stmt(this, psz, m_bThrowExceptions);
	if(stmt.next())
		return stmt.getInt64(0, nDefault);
	return nDefault;
}

double Connection::getDouble(const char *psz, double dDefault) {
	Statement stmt(this, psz, m_bThrowExceptions);
	if(stmt.next())
		return stmt.getDouble(0, dDefault);
	return dDefault;
}

std::string Connection::getText(const char *psz, const char *pszDefault) {
	Statement stmt(this, psz, m_bThrowExceptions);
	if(stmt.next())
		return stmt.getText(0, pszDefault);
	return pszDefault;
}

int64_t Connection::getRowid() {
	return sqlite3_last_insert_rowid(m_pConn);
}

int Connection::getChangeCount() {
	return sqlite3_changes(m_pConn);
}

Statement::Statement(Connection *pConn, const std::string &str, bool bExceptions) : m_pConn(pConn), m_lock(pConn->m_lock), m_pStmt(NULL), m_bThrowExceptions(bExceptions) {
	int nResult = sqlite3_prepare_v2(m_pConn->m_pConn, str.c_str(), -1, &m_pStmt, NULL);
	m_lock.unlock();
	if(nResult != SQLITE_OK)
		m_pConn->handleError(nResult, "Failed to prepare statement", m_bThrowExceptions);
}

Statement::Statement(Connection *pConn, const char *psz, bool bExceptions) : m_pConn(pConn), m_lock(pConn->m_lock), m_pStmt(NULL), m_bThrowExceptions(bExceptions) {
	int nResult = sqlite3_prepare_v2(m_pConn->m_pConn, psz, -1, &m_pStmt, NULL);
	m_lock.unlock();
	if(nResult != SQLITE_OK)
		m_pConn->handleError(nResult, "Failed to prepare statement", m_bThrowExceptions);
}

Statement::~Statement() {
	m_lock.lock(); // This will be unlocked as soon as the destructor goes out of scope
	if(m_pStmt) {
		try { sqlite3_finalize(m_pStmt); } catch(...) { VKLogError("Failed to close statement handle"); }
		m_pStmt = NULL;
	}
}

bool Statement::reset(bool bLogError) {
	int nResult = sqlite3_reset(m_pStmt);
	m_lock.unlock();
	if(nResult != SQLITE_OK) {
		if(bLogError)
			m_pConn->handleError(nResult, "Failed to reset statement", m_bThrowExceptions);
		return false;
	}
	return true;
}

bool Statement::exec(bool bLogError) {
	m_lock.lock();
	int nResult = sqlite3_step(m_pStmt);
	if(bLogError && nResult != SQLITE_DONE)
		m_pConn->handleError(nResult, "Failed to exec statement", m_bThrowExceptions);
	return reset(bLogError); // Will log any errors
}

bool Statement::next(bool bLogError) {
	m_lock.lock();
	int nResult = sqlite3_step(m_pStmt);
	if(nResult == SQLITE_ROW)
		return true;
	if(nResult == SQLITE_DONE)
		return false;
	if(bLogError)
		m_pConn->handleError(nResult, "Failed to fetch from statement", m_bThrowExceptions);
	return false;
}

int64_t Statement::insert(bool bLogError) {
	int64_t rowid = -1;
	m_lock.lock();
	int nResult = sqlite3_step(m_pStmt);
	if(nResult == SQLITE_DONE)
		rowid = m_pConn->getRowid();
	reset(bLogError); // Will log any errors
	return rowid;
}

int Statement::update(bool bLogError) {
		int nChanges = -1;
	m_lock.lock();
	int nResult = sqlite3_step(m_pStmt);
	if(nResult == SQLITE_DONE)
		nChanges = m_pConn->getChangeCount();
	reset(bLogError); // Will log any errors
	return nChanges;
}

bool Statement::bind(int n) {
	int nResult = ::sqlite3_bind_null(m_pStmt, n);
	if(nResult == SQLITE_OK)
		return true;
	m_pConn->handleError(nResult, "Failed to bind column", m_bThrowExceptions);
	return false;
}

bool Statement::bind(int n, const char *pszValue, int nLength) {
	int nResult = pszValue ?
		::sqlite3_bind_text(m_pStmt, n, pszValue, nLength, NULL) :
		::sqlite3_bind_null(m_pStmt, n);
	if(nResult == SQLITE_OK)
		return true;
	m_pConn->handleError(nResult, "Failed to bind column", m_bThrowExceptions);
	return false;
}

bool Statement::bind(int n, int nValue) {
	int nResult = ::sqlite3_bind_int(m_pStmt, n, nValue);
	if(nResult == SQLITE_OK)
		return true;
	m_pConn->handleError(nResult, "Failed to bind column", m_bThrowExceptions);
	return false;
}

bool Statement::bind(int n, int64_t nValue) {
	int nResult = ::sqlite3_bind_int64(m_pStmt, n, nValue);
	if(nResult == SQLITE_OK)
		return true;
	m_pConn->handleError(nResult, "Failed to bind column", m_bThrowExceptions);
	return false;
}

bool Statement::bind(int n, double dValue) {
	int nResult = ::sqlite3_bind_double(m_pStmt, n, dValue);
	if(nResult == SQLITE_OK)
		return true;
	m_pConn->handleError(nResult, "Failed to bind column", m_bThrowExceptions);
	return false;
}

int Statement::type(int n) {
	return ::sqlite3_column_type(m_pStmt, n);
}

const char *Statement::name(int n) {
	return ::sqlite3_column_name(m_pStmt, n);
}

int Statement::getInt(int n, int nDefault) {
	if(sqlite3_column_type(m_pStmt, n) == SQLITE_NULL)
		return nDefault;
	return sqlite3_column_int(m_pStmt, n);
}

int64_t Statement::getInt64(int n, int64_t nDefault) {
	if(sqlite3_column_type(m_pStmt, n) == SQLITE_NULL)
		return nDefault;
	return sqlite3_column_int64(m_pStmt, n);
}

double Statement::getDouble(int n, double dDefault) {
	if(sqlite3_column_type(m_pStmt, n) == SQLITE_NULL)
		return dDefault;
	return sqlite3_column_double(m_pStmt, n);
}

const char *Statement::getText(int n, const char *pszDefault) {
	if(sqlite3_column_type(m_pStmt, n) == SQLITE_NULL)
		return pszDefault;
	return (const char *)sqlite3_column_text(m_pStmt, n);
}

} // namespace DB
} // namespace VK

