// VKDatabase.h
//

#ifndef __VKDatabase_h__
#define __VKDatabase_h__

#include "VKThread.h"

// Forward reference
struct sqlite3;
struct sqlite3_stmt;

namespace VK {
namespace DB {

inline std::string quote(const char *psz) {
	char szBuffer[1024];
	int i = 0;
	szBuffer[i++] = '\'';
	while(i < 1022 && *psz) {
		szBuffer[i++] = *psz;
		if(*psz == '\'') szBuffer[i++] = *psz;
		psz++;
	}
	szBuffer[i++] = '\'';
	szBuffer[i++] = 0;
	return szBuffer;
}

struct Callback {
	virtual int handleRow(int nColumns, char **pValues,char **pNames) = 0;
};

class Connection {
	friend class Statement;

private:
	sqlite3 *m_pConn;

protected:
	Thread::Lock m_lock;
	Path m_path;
	std::string m_strLastError;
	bool m_bThrowExceptions, m_bInTransaction;

	void handleError(const char *pszMessage, bool bException);
	void handleError(int nCode, const char *pszMessage, bool bException);
	static int static_callback(void *pCallback, int nColumns, char **pValues,char **pNames);

public:
	enum {
		ReadOnly = 0x00000001,
		ReadWrite = 0x00000002,
		Create = 0x00000004,
	};

	Connection(bool bThrowExceptions=true) : m_pConn(NULL), m_bThrowExceptions(bThrowExceptions), m_bInTransaction(false) {}
	~Connection() { close(); }

	void setExceptions(bool b) { m_bThrowExceptions = b; }

	bool open(Path path, int nFlags = Create | ReadWrite, int nTimeout = 15000);
	void close();
	bool exec(const char *psz, Callback *pCallback=NULL, bool bLogError=true);
	int64_t insert(const char *psz);
	int getInt(const char *psz, int nDefault=-1);
	int64_t getInt64(const char *psz, int64_t nDefault=-1);
	double getDouble(const char *psz, double dDefault=-1.0);
	std::string getText(const char *psz, const char *pszDefault="");
	int64_t getRowid();
	int getChangeCount();
	bool backup(Path path);

	bool exec(const std::string &str, Callback *pCallback=NULL, bool bLogError=true) { return exec(str.c_str(), pCallback, bLogError); }
	int64_t insert(const std::string &str) { return insert(str.c_str()); }
	int getInt(const std::string &str, int nDefault=-1) { return getInt(str.c_str(), nDefault); }
	int64_t getInt64(const std::string &str, int64_t nDefault=-1) { return getInt64(str.c_str(), nDefault); }
	double getDouble(const std::string &str, double dDefault=-1.0) { return getDouble(str.c_str(), dDefault); }
	std::string getText(const std::string &str, const char *pszDefault="") { return getText(str.c_str(), pszDefault); }
	//std::string getString(const std::string &str, const char *pszDefault="") { return getText(str.c_str(), pszDefault); }

	bool isOpen() { return m_pConn != NULL; }
	bool isInTransaction() { return m_bInTransaction; }
	const Path &getPath() { return m_path; }
	const std::string &getLastError() { return m_strLastError; }
	const Thread::Lock &getLock() { return m_lock; }
	
	bool beginTransaction() {
		Thread::AutoLock lock(m_lock);
		if(m_bInTransaction)
			return false;
		exec("BEGIN DEFERRED TRANSACTION;");
		m_bInTransaction = true;
		return true;
	}

	bool commitTransaction() {
		Thread::AutoLock lock(m_lock);
		if(!m_bInTransaction)
			return false;
		exec("COMMIT TRANSACTION;");
		m_bInTransaction = false;
		return true;
	}

	bool rollbackTransaction() {
		Thread::AutoLock lock(m_lock);
		if(!m_bInTransaction)
			return false;
		m_bThrowExceptions = false;
		bool bTemp = m_bThrowExceptions;
		exec("ROLLBACK TRANSACTION;");
		m_bThrowExceptions = bTemp;
		m_bInTransaction = false;
		return true;
	}

};

class Statement {
private:
	sqlite3_stmt *m_pStmt;
	Thread::AutoLock m_lock;

protected:
	Connection *m_pConn;
	bool m_bThrowExceptions;

public:
	Statement(Connection *pConn, const std::string &str, bool bExceptions=true);
	Statement(Connection *pConn, const char *psz, bool bExceptions=true);
	~Statement();

	void setExceptions(bool b) { m_bThrowExceptions = b; }

	bool reset(bool bLogError=true); // Call to reset a prepared statement so it can be executed again
	bool exec(bool bLogError=true); // Call to exec a prepared statement that does NOT return a row (calls reset automatically)
	bool next(bool bLogError=true); // Call to exec a prepared statement that returns a row
	int64_t insert(bool bLogError=true); // Call instead of exec() when a rowid needs to be returned
	int update(bool bLogError=true); // Call instead of exec() when a record count needs to be returned

	// Call to bind parameters before executing
	bool bind(int n);
	bool bind(int n, int nValue);
	bool bind(int n, int64_t nValue);
	bool bind(int n, double dValue);
	bool bind(int n, const char *pszValue, int nLength=-1);
	bool bind(int n, unsigned int nValue) { return bind(n, (int64_t)nValue); }
	bool bind(int n, uint64_t nValue) { return bind(n, (int64_t)nValue); }
	bool bind(int n, const std::string &str) { return bind(n, str.c_str(), (int)str.length()); }

	bool bind_null(int n, int nValue, int nNull=-1) {
		if(nValue == nNull) return bind(n);
		return bind(n, nValue);
	}
	bool bind_null(int n, const char *psz, int nLength = -1) { 
		if(!psz || !psz[0]) return bind(n);
		return bind(n, psz, nLength);
	}
	bool bind_null(int n, const std::string &str) { 
		if(str.empty()) return bind(n);
		return bind(n, str.c_str(), (int)str.length());
	}

	// Call to get values from a result set
	int type(int n);
	const char *name(int n);
	int getInt(int n, int nDefault=-1);
	int64_t getInt64(int n, int64_t nDefault=-1);
	double getDouble(int n, double dDefault=-1.0);
	const char *getText(int n, const char *pszDefault=NULL);
};

} // namespace DB
} // namespace VK

#endif // __VKDatabase_h__
