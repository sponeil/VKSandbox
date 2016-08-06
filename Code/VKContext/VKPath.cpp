// VKPath.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKPath.h"
extern "C" {
	#include <zip.h>
}

#ifdef _WIN32
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace VK {

#ifdef _WIN32
// The Microsoft FILETIME is in 100-nanosecond intervals since 1601/01/01
inline double FileTimeToEpoch(const FILETIME &ft) {
	return (((uint64_t)ft.dwHighDateTime << 32) + (__int64)ft.dwLowDateTime - 116444736000000000L) / 10000000.0;
}
#endif

#ifdef _WIN32
const char *Path::SEPARATOR = "\\";
const char *Path::ALT_SEPARATOR = "/";
#else
#include<dirent.h>
const char *Path::SEPARATOR = "/";
const char *Path::ALT_SEPARATOR = "\\";
#endif

#ifdef ANDROID
Path Path::m_pathAPK;
Path Path::m_pathInternal;
Path Path::m_pathExternal;
#endif


std::string Path::basename(const char *pszSuffix) const {
	int nSuffix = 0; // The number of suffix bytes to strip off the end
	if(pszSuffix && *pszSuffix) {
		nSuffix = (int)strlen(pszSuffix);
#ifdef _WIN32
		if(_memicmp(pszSuffix, &m_strPath[m_strPath.length() - nSuffix], nSuffix) != 0)
#else
		if(memcmp(pszSuffix, &m_strPath[m_strPath.length() - nSuffix], nSuffix) != 0)
#endif
			nSuffix = 0;
	}
	std::string::size_type sep = m_strPath.rfind(SEPARATOR_CHAR);
	std::string::size_type alt_sep = m_strPath.rfind(ALT_SEPARATOR_CHAR);
	if(alt_sep != std::string::npos && (sep == std::string::npos || alt_sep > sep))
		sep = alt_sep;
	if(sep == std::string::npos)
		return nSuffix == 0 ? m_strPath : m_strPath.substr(0, m_strPath.length() - nSuffix);
	return m_strPath.substr(sep+1, m_strPath.length() - (sep+1 + nSuffix));
}

std::string Path::extension() const {
	std::string base = basename();
	std::string::size_type pos = base.rfind('.');
	if(pos == std::string::npos)
		return "";
	return std::string(&base[pos+1]);
}

Path Path::dirname() const {
	std::string::size_type sep = m_strPath.rfind(SEPARATOR_CHAR);
	std::string::size_type alt_sep = m_strPath.rfind(ALT_SEPARATOR_CHAR);
	if(alt_sep != std::string::npos && (sep == std::string::npos || alt_sep > sep))
		sep = alt_sep;
	if(sep == std::string::npos)
		return ".";
	return m_strPath.substr(0, sep);
}

Path Path::expand_path() const {
	char szPath[_MAX_PATH];
#ifdef _WIN32
	::GetFullPathName(m_strPath.c_str(), MAX_PATH, szPath, NULL);
#else
    ::realpath(m_strPath.c_str(), szPath);
#endif
	return szPath;
}

Path Path::add(const char *pszFormat, ...) const {
	char szBuffer[_MAX_PATH];
	va_list va;
	va_start(va, pszFormat);
	vsnprintf(szBuffer, _MAX_PATH, pszFormat, va);
	szBuffer[_MAX_PATH-1] = 0;
	va_end(va);
	return *this + szBuffer;
}

// Per MS, this is how to change a Unix time to a FILETIME
// (Reverse it to go from FILETIME to Unix time)
//void UnixTimeToFileTime(time_t t, LPFILETIME pft) {
//	LONVKONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
//	pft->dwLowDateTime = (DWORD)ll;
//	pft->dwHighDateTime = ll >> 32;
//}

bool Path::exists() {
	WIN32_FILE_ATTRIBUTE_DATA data;
	if(::GetFileAttributesEx(m_strPath.c_str(), GetFileExInfoStandard, &data) == 0) {
		m_nAttributes = 0;
		m_nSize = (uint64_t)-1;
		m_dCreated = m_dLastWrite = m_dLastAccess = 0.0;
		return false;
	}

	m_nAttributes = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DirectoryType : FileType;
	m_nSize = ((uint64_t)data.nFileSizeHigh << 32) + (uint64_t)data.nFileSizeLow;
	m_dCreated = FileTimeToEpoch(data.ftCreationTime);
	m_dLastWrite = FileTimeToEpoch(data.ftLastWriteTime);
	m_dLastAccess = FileTimeToEpoch(data.ftLastAccessTime);
	return true;
}

Line::List Path::readlines() const {
	char szBuffer[16384];
	Line::List lines;
	std::ifstream in(m_strPath.c_str());
	in.getline(szBuffer, 16384);
	while(in) {
		lines.push_back(szBuffer);
		in.getline(szBuffer, 16384);
	}
	return lines;
}

std::string Path::read(unsigned int n) const {
	std::ifstream in(m_strPath.c_str(), std::ios::binary);
	if(n == 0) {
		in.seekg(0, std::ios::end);
		n = (unsigned int)in.tellg();
		in.seekg(0, std::ios::beg);
	}
	std::string str;
	str.resize(n);
	in.read(&str[0], n);
	if((unsigned int)in.gcount() < n)
		str.resize((unsigned int)in.gcount());
	return str;
}

#ifdef ANDROID
Line::List Path::apk_readlines() const {
	Line::List lines;
	char szBuffer[16384];
	zip *pZip = ::zip_open(m_pathAPK, 0, NULL);
	if(!pZip) {
		VKLogError("Failed to open apk: ", (const char *)m_pathAPK);
		return lines;
	}
	
	struct zip_stat st;
	if(zip_stat(pZip, m_strPath.c_str(), 0, &st) < 0) {
		VKLogError("Failed to find file in apk: ", m_strPath.c_str());
	} else {
		zip_file *pFile = zip_fopen_index(pZip, st.index, 0);
		if(!pFile) {
			VKLogError("Failed to open file in apk: ", m_strPath.c_str());
		} else {
			std::string str;
			str.resize(st.size);
			unsigned int nRead = zip_fread(pFile, &str[0], st.size);
			if(nRead != st.size) {
				VKLogError("Failed to read %d bytes in %s (%d returned)", st.size, m_strPath.c_str(), nRead);
				str.clear();
			} else {
				const char *psz = str.c_str();
				while(*psz) {
					const char *pszEnd = strchr(psz, '\n');
					if(!pszEnd) break;
					lines.push_back(std::string(psz, (unsigned int)(pszEnd-psz)));
					psz = pszEnd+1;
				}
			}
			zip_fclose(pFile);
		}
	}
	zip_close(pZip);
	return lines;
}

std::string Path::apk_read(unsigned int n) const {
	zip *pZip = ::zip_open(m_pathAPK, 0, NULL);
	if(!pZip) {
		VKLogError("Failed to open apk: ", (const char *)m_pathAPK);
		return "";
	}

	std::string str;
	struct zip_stat st;
	if(zip_stat(pZip, m_strPath.c_str(), 0, &st) < 0) {
		VKLogError("Failed to find file in apk: ", m_strPath.c_str());
	} else {
		if(n == 0 || n > st.size) n = st.size;
		zip_file *pFile = zip_fopen_index(pZip, st.index, 0);
		if(!pFile) {
			VKLogError("Failed to open file in apk: ", m_strPath.c_str());
		} else {
			str.resize(n);
			unsigned int nRead = zip_fread(pFile, &str[0], n);
			if(nRead != n) {
				VKLogError("Failed to read %d bytes in %s (%d returned)", n, m_strPath.c_str(), nRead);
				str.clear();
			}
			zip_fclose(pFile);
		}
	}
	zip_close(pZip);
	return str;
}
#endif

bool Path::chdir() const {
#ifdef _WIN32
    return ::SetCurrentDirectory(m_strPath.c_str()) != 0;
#else
	return ::chdir(m_strPath.c_str()) == 0;
#endif
}

bool Path::del() {
	if(exists()) {
		if(directory())
			return rmdir();
		else
#ifdef _WIN32
			return ::DeleteFile(m_strPath.c_str()) != 0;
#else
            return ::remove(m_strPath.c_str()) == 0;
#endif
	}
	return true;
}

bool Path::mkdir() {
	if(exists())
		return directory();
#ifdef _WIN32
	return ::CreateDirectory(m_strPath.c_str(), NULL) != 0;
#else
	return ::mkdir(m_strPath.c_str(), 0777) == 0;
#endif
}

bool Path::rmdir() {
	if(exists() && directory())
#ifdef _WIN32
		return ::RemoveDirectory(m_strPath.c_str()) != 0;
#else
        return ::rmdir(m_strPath.c_str()) == 0;
#endif
	return true;
}

#ifdef ANDROID
Path::List Path::apk_entries(PathType type) const {
	Path::List list;
	zip *pZip = ::zip_open(m_pathAPK, 0, NULL);
	if(!pZip) {
		VKLogError("Failed to open apk: ", (const char *)m_pathAPK);
		return list;
	}
	
	//Just for debug, print APK contents
	int numFiles = zip_get_num_files(pZip);
	for(int i=0; i<numFiles; i++) {
		const char* name = zip_get_name(pZip, i, 0);
		if(!name) {
			VKLogError("Error reading zip file name at index %i : %s", zip_strerror(pZip));
			break;
		}
		if(memcmp(name, m_strPath.c_str(), m_strPath.length()) == 0 &&
		   (name[m_strPath.length()] == SEPARATOR_CHAR || name[m_strPath.length()] == ALT_SEPARATOR_CHAR)) {
			std::string str = name;
			const char *pszEnd = strchr(str.c_str()+(m_strPath.length()+1), SEPARATOR_CHAR);
			if(!pszEnd) pszEnd = strchr(str.c_str()+(m_strPath.length()+1), ALT_SEPARATOR_CHAR);
			if(pszEnd) str.resize((size_t)(pszEnd-str.c_str()));
			Path p(str);
			p.m_nAttributes = (pszEnd != NULL) ? DirectoryType : FileType;
			if((p.m_nAttributes & type) != 0) {
				struct zip_stat st;
				if(zip_stat_index(pZip, i, 0, &st) == 0) {
					p.m_nSize = st.size;
					p.m_dLastAccess = p.m_dLastWrite = p.m_dCreated = st.mtime;
				}
				list.push_back(p);
			}
		}
	}
	zip_close(pZip);
}
#endif

Path::List Path::entries(const char *pszMask, PathType type) const {
	Path::List list;
#ifdef _WIN32
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(*this + pszMask, &fd);
	if(hFind != INVALID_HANDLE_VALUE) {
		do {
			if(strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
				continue;
			Path p = *this + fd.cFileName;
			p.m_nAttributes = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? DirectoryType : FileType;
			p.m_nSize = ((uint64_t)fd.nFileSizeHigh << 32) + (uint64_t)fd.nFileSizeLow;
			p.m_dCreated = FileTimeToEpoch(fd.ftCreationTime);
			p.m_dLastWrite = FileTimeToEpoch(fd.ftLastWriteTime);
			p.m_dLastAccess = FileTimeToEpoch(fd.ftLastAccessTime);
			if((p.m_nAttributes & type) != 0)
				list.push_back(p);
		} while(::FindNextFile(hFind, &fd));
		::FindClose(hFind);
		std::sort(list.begin(), list.end(), EntrySort);
	}
#else
	DIR *dirp = opendir(this->c_str());
	if(dirp != NULL) {
		struct dirent *dptr;
		while(dptr = readdir(dirp)) {
			if(*dptr->d_name == '.')
				continue;
			Path p(*this + dptr->d_name);
			if(p.exists() && ((p.m_bDir && type != FileType) || (!p.m_bDir && type != DirectoryType))) {
				p.m_strPath = dptr->d_name;
				list.push_back(p);
			}
		}
		closedir(dirp);
	}
#endif
	return list;
}

Path Path::Getwd() {
	char szPath[_MAX_PATH];
#ifdef _WIN32
	::GetCurrentDirectory(_MAX_PATH, szPath);
#else
    ::getcwd(szPath, _MAX_PATH);
#endif
	return Path(szPath);
}

#ifdef _WIN32
Path Path::Module() {
	char szPath[_MAX_PATH];
	::GetModuleFileName(NULL, szPath, _MAX_PATH);
	return Path(szPath).dirname();
}
#endif

} // namespace VK
