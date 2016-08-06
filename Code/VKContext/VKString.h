// VKString.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKString_h__
#define __VKString_h__

namespace VK {
namespace String {

/// A simple parser used to parse GLFX files.
/// It allocates one buffer and does all of its work in-place. So if you tell
/// it to strip "/*" to "*/" blocks, it finds them and uses memmove() to
/// shrink the string (which is the most efficient way to do it). Some methods
/// will shrink the string, but none will ever grow it. So features like
/// #include substitution must be handled another way (i.e. ostringstream).
class Parser {
protected:
	unsigned int m_nIndex;
	unsigned int m_nLength;
	char *m_psz;

public:
	Parser() : m_nIndex(0), m_nLength(0), m_psz(NULL) {}
	~Parser() { delete m_psz; }
	Parser(const Parser &p) { init(p); }
	Parser(const char *psz, unsigned int nLength=0) { init(psz, nLength); }
	Parser(const std::string &str) { init(str); }
	void operator=(const Parser &p) { init(p); }

	void init(const std::string &str) {
		m_nIndex = 0;
		m_nLength = (unsigned int)str.size();
		m_psz = new char[m_nLength+1];
		memcpy(m_psz, str.c_str(), m_nLength+1);
	}

	void init(const char *psz, unsigned int nLength=0) {
		m_nIndex = 0;
		m_nLength = nLength > 0 ? nLength : (unsigned int)strlen(psz);
		m_psz = new char[m_nLength+1];
		memcpy(m_psz, psz, m_nLength+1);
	}

	void init(const Parser &p) {
		m_nIndex = 0;
		m_nLength = p.getLength();
		if(m_nLength > 0) {
			m_psz = new char[m_nLength+1];
			memcpy(m_psz, p.getBuffer(), m_nLength+1);
		} else
			m_psz = NULL;
	}

	const char *getBuffer() const { return m_psz; }
	unsigned int getLength() const { return m_nLength; }
	unsigned int getIndex() const { return m_nIndex; }
	void resetIndex() { m_nIndex = 0; }

	void stripBlocks(const char *pszStart, const char *pszEnd, std::vector<Parser> *pBlocks = NULL) {
		unsigned int nEndLength = (unsigned int)strlen(pszEnd);
		while(char *pStart = strstr(m_psz, pszStart)) {
			char *pEnd = strstr(pStart, pszEnd);
			pEnd = pEnd ? pEnd + nEndLength : m_psz + m_nLength;
			if(pEnd[-1] == '\n') --pEnd; // Avoid concatenating lines together
			unsigned int nSkip = (unsigned int)(pEnd - pStart);
			unsigned int nStart = (unsigned int)(pEnd - m_psz);
			if(nStart < m_nLength) {
				if(pBlocks) {
					pBlocks->push_back(Parser()); // Sometimes STL sucks sooo bad
					pBlocks->back().init(pStart, nSkip);
				}
				memmove(pStart, pEnd, m_nLength-nStart);
			}
			m_nLength -= nSkip;
			m_psz[m_nLength] = 0;
		}
	}

	typedef std::pair<char *, unsigned int> Line;
	void getLines(std::vector<Line> &vLines) {
		char *pszStart = m_psz;
		while(*pszStart) {
			// Find the end of the next line
			char *pszEnd = strchr(pszStart, '\n');
			pszEnd = pszEnd ? pszEnd+1 : pszStart+strlen(pszStart);
			unsigned int nLength = (unsigned int)(pszEnd-pszStart);

			// Strip trailing whitespace
			while(nLength > 0 && pszStart[nLength-1] <= ' ')
				--nLength;

			// If there's anything left, add it to the list
			if(nLength > 0)
				vLines.push_back(std::pair<char *, unsigned int>(pszStart, nLength));

			// Skip over the line we just processed
			pszStart = pszEnd;
		}
	}

	unsigned int nextDelimiter(char *pszDest, char cDelimiter) {
		// Skip leading whitespace
		while(m_psz[m_nIndex] != 0 && m_psz[m_nIndex] <= ' ')
			++m_nIndex;

		// Grab up to 255 characters or until we reach cDelimiter or end of string
		unsigned int nLength = 0;
		while(m_psz[m_nIndex] != 0 && m_psz[m_nIndex] != cDelimiter && nLength < 255) {
			*pszDest++ = m_psz[m_nIndex++];
			++nLength;
		}

		*pszDest = 0; // NULL-terminate the string
		return nLength; // Return the length
	}

	unsigned int nextCodeBlock(char *pszDest, unsigned int nMax=16384, char cOpen = '{', char cClose = '}') {
		// Skip leading whitespace
		while(m_psz[m_nIndex] != 0 && m_psz[m_nIndex] <= ' ')
			++m_nIndex;

		// Grab up to nMax characters or until we reach cDelimiter or end of string
		unsigned int nLength = 0;
		if(m_psz[m_nIndex] == cOpen) {
			int nLevel = 1;
			*pszDest++ = m_psz[m_nIndex++];
			++nLength;
			while(m_psz[m_nIndex] != 0 && nLevel > 0 && nLength < nMax) {
				if(m_psz[m_nIndex] == cOpen)
					nLevel++;
				else if(m_psz[m_nIndex] == cClose)
					nLevel--;
				*pszDest++ = m_psz[m_nIndex++];
				++nLength;
			}
		}

		*pszDest = 0; // NULL-terminate the string
		return nLength; // Return the length
	}

	unsigned int nextToken(char *pszDest) {
		// Skip leading whitespace
		while(m_psz[m_nIndex] != 0 && m_psz[m_nIndex] <= ' ')
			++m_nIndex;

		unsigned int nLength = 0;
		char c = m_psz[m_nIndex];

		// Does it start with a character that indicates it's a name?
		if(c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
			// Grab up to 255 characters or until we reach an illegal character to have in a name
			while(nLength < 255 && (c == '_' || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
				*pszDest++ = c;
				c = m_psz[++m_nIndex];
				++nLength;
			}
		} else if(c != 0) { // If it's not a name, assume it is a 1-byte symbol
			*pszDest++ = c;
			++m_nIndex;
			nLength = 1;
		}

		*pszDest = 0; // NULL-terminate the string
		return nLength; // Return the length
	}

};

} // namespace String
} // namespace VK

#endif // __VKString_h__
