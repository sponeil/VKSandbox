// VKPath.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKPath_h__
#define __VKPath_h__

namespace VK {

// Allows Path.readlines() to return a Line::List
namespace Line { typedef std::vector<std::string> List; }

/// A simple helper class for handling paths and files.
/// In addition to making it easy to use relative paths to find
/// files, it is also used to get sub-directory and file listings.
/// It can easily be expanded to do a lot more.
class Path
{
public:
	typedef std::vector<Path> List;
	enum PathType {NoType = 0, AllTypes = 0, FileType = 1, DirectoryType = 2};

	static const char *SEPARATOR;
	static const char *ALT_SEPARATOR;
#ifdef _WIN32
	enum { SEPARATOR_CHAR = '\\', ALT_SEPARATOR_CHAR = '/' };
#else
	enum { SEPARATOR_CHAR = '/', ALT_SEPARATOR_CHAR = '\\' };
#endif

protected:
	static bool EntrySort(const Path &p1, const Path &p2) { return strcmp(p1.m_strPath.c_str(), p2.m_strPath.c_str()) < 0; }
	static std::string Trim(const char *psz, int n=-1) {
		if(n < 0) n = psz != NULL ? (int)strlen(psz) : 0;
		if(n == 0) return "";
		while(n > 0 && (psz[n-1] == SEPARATOR_CHAR || psz[n-1] == ALT_SEPARATOR_CHAR))
			--n;
		return std::string(psz, n);
	}
	static std::string Trim(const std::string &str) {
		return Trim(str.c_str(), (int)str.length());
	}

#ifdef ANDROID
	static Path m_pathAPK;
	static Path m_pathInternal;
	static Path m_pathExternal;
#endif
	
	std::string m_strPath;	///< String containing the current path (trailing slashes are always trimmed automatically)
	//int m_nTime;			///< The last time this path was changed in Unix time (only set by certain methods)
	//bool m_bDir;			///< Set to true if the path points to a directory, false otherwise (only set by certain methods)

	uint32_t m_nAttributes;	///< The attributes of this path (only set by certain methods)
	uint64_t m_nSize;		///< If the path points to a file, this contains the file size (only set by certain methods)
	double m_dCreated;		///< The time this path was created (only set by certain methods)
	double m_dLastWrite;	///< The last time this path was modified (only set by certain methods)
	double m_dLastAccess;	///< The last time this path was accessed (only set by certain methods)

public:
	/// @name Constructors
	//@{
	Path() : m_nAttributes(0), m_nSize((uint64_t)-1) {}
	Path(const std::string &str) : m_nAttributes(0), m_nSize((uint64_t)-1) { *this = str; }
	Path(const char *psz) : m_nAttributes(0), m_nSize((uint64_t)-1) { *this = psz; }
	Path(const Path &path) : m_nAttributes(0), m_nSize((uint64_t)-1) { *this = path; }
	//@}

	/// @name Assignment and casting operators
	//@{
	const Path &operator=(const std::string &str)	{ m_strPath = Trim(str); return *this; }
	const Path &operator=(const char *psz)		{ m_strPath = Trim(psz); return *this; }
	const Path &operator=(const Path &path)		{
		m_strPath = path.m_strPath;
		m_nAttributes = path.m_nAttributes;
		m_nSize = path.m_nSize;
		m_dCreated = path.m_dCreated;
		m_dLastWrite = path.m_dLastWrite;
		m_dLastAccess = path.m_dLastAccess;
		return *this;
	}

	operator const char *() const					{ return m_strPath.c_str(); }
	//const char *operator &() const				{ return m_strPath.c_str(); }
	bool operator==(const Path &path) const {
		return m_dLastWrite == path.m_dLastWrite && m_nAttributes == path.m_nAttributes && m_nSize == path.m_nSize && m_strPath == path.m_strPath;
	}
	//@}

	const std::string &str() const				{ return m_strPath; }
	const char *c_str() const					{ return m_strPath.c_str(); }
	size_t length() const						{ return m_strPath.length(); }

	/// @name Path concatenating operators
	//@{
	void operator+=(const std::string &str) { if(!str.empty()) m_strPath = Trim(m_strPath + SEPARATOR + str); }
	void operator+=(const char *psz) { if(psz && *psz) m_strPath = Trim(m_strPath + SEPARATOR + psz); }
	Path operator+(const std::string &str) const { return !str.empty() ? Path(m_strPath + SEPARATOR + str) : *this; }
	Path operator+(const char *psz) const { return psz && *psz ? Path(m_strPath + SEPARATOR + psz) : *this; }
	Path add(const char *pszFormat, ...) const;
	//@}

	// Simple Ruby File methods
	std::string extension() const;
	std::string basename(const char *pszSuffix=NULL) const;
	Path dirname() const;
	Path expand_path() const;
	Path join(const std::string &str) const { return str.empty() ? m_strPath : Trim(m_strPath + SEPARATOR + str); }
	Path join(const char *psz) const { return !psz || !*psz ? m_strPath : Trim(m_strPath + SEPARATOR + psz); }

	// Simple Ruby FileTest methods
	bool exists();	///< This method retrieves and caches info about the current path. It returns true if the path exists.
	bool file() const { return (m_nAttributes & FileType) != 0; }	///< Returns true if the current path exists and is not a directory (uses cached info)
	bool directory() const { return (m_nAttributes & DirectoryType) != 0; }	///< Returns true if the current path exists and is a directory (uses cached info)
	uint64_t size() const { return m_nSize; }	///< Returns the size of the current file (uses cached info)
	double ctime() const { return m_dCreated; }	///< Returns the file's creation time in seconds since epoch (uses cached info)
	double mtime() const { return m_dLastWrite; }	///< Returns the file's last modify time in seconds since epoch (uses cached info)
	double atime() const { return m_dLastAccess; }	///< Returns the file's last access time in seconds since epoch (uses cached info)

	// Simple Ruby IO methods
	Line::List readlines() const;
	std::string read(unsigned int n = 0) const;

	bool hasChanged(bool bCheckDeleted=false) const {
		Path p = *this;
		return (p.exists() || bCheckDeleted) ?
			abs(p.m_dLastWrite - m_dLastWrite) < 0.0001 : // == not safe with floating-point
			false;
	}

	// Simple Ruby Dir methods
	bool chdir() const;
	bool del();
	bool mkdir();
	bool rmdir();
	List entries(const char *pszMask = "*.*", PathType type=AllTypes) const;
	List files(const char *pszMask = "*.*") const { return entries(pszMask, FileType); }
	List directories(const char *pszMask = "*.*") const { return entries(pszMask, DirectoryType); }

#ifdef ANDROID
	List apk_entries(PathType type=AllTypes) const;
	List apk_files() const { return apk_entries(FileType); }
	List apk_directories() const { return apk_entries(DirectoryType); }
	Line::List apk_readlines() const;
	std::string apk_read(unsigned int n = 0) const;
#endif

	static Path Getwd();
	static Path Pwd() { return Getwd(); }

#ifdef _WIN32
	static Path Module();
	static Path Root() { return Module().dirname(); }
#endif

#ifdef ANDROID
	static void Init(const char *apk, const char *internal, const char *external) {
		if(apk == NULL) apk = "";
		if(internal == NULL) internal = "";
		if(external == NULL) external = "";
		m_pathAPK = apk;
		m_pathInternal = internal;
		m_pathExternal = external;
	}
	static Path APK() { return m_pathAPK; }
	static Path Internal() { return m_pathInternal; }
	static Path External() { return m_pathExternal; }
	static Path Root() { return "assets"; }
#endif
	
	static Path Font() { return Root() + "fonts"; }
	static Path Shader() { return Root() + "shaders"; }
	static Path Images() { return Root() + "images"; }
	static Path Log() { return Root() + "log"; }
};


} // namespace VK

#endif // __VKPath_h__
