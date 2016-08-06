// VKShaderFile.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKShaderFile.h"

namespace VK {


bool ShaderFile::load(const Path &info) {
	if(m_info == info)
		return false;
	m_info = info;

	VKLogInfo("Loading %s", info.c_str());
#ifdef ANDROID
	m_strContents = info.apk_read();
#else
	m_strContents = info.read();
#endif
	if(m_strContents.empty() && info.size() > 0) {
		VKLogError("Failed to read shader file: %s", m_info.c_str());
		return false;
	}
	return true;
}

bool FXFile::parse(Context &vk, std::map<std::string, ShaderFile> &mapShaderFiles, std::map<std::string, ShaderTechnique> &mapTechniques) {
	VKLogInfo("FXFile::parse(%s)", m_info.c_str());
	// Clear the dependency list
	clear_dependencies();

	// Copy the VKFX file contents to a String::Parser and start parsing it
	String::Parser parser(getContents());
	std::vector<String::Parser> vTechniqueBlocks;
	vTechniqueBlocks.reserve(10);
	parser.stripBlocks("//", "\n"); // Strip single-line comment blocks
	parser.stripBlocks("/*", "*/"); // Strip multi-line comment blocks
	parser.stripBlocks("technique", "\n}", &vTechniqueBlocks); // Strip technique blocks, saving them for later

	// Break what's left up into lines (automatically strips trailing whitespace from each)
	std::vector<String::Parser::Line> vLines;
	vLines.reserve(1024);
	parser.getLines(vLines);

	// Build a string of what's left (substituting #include's)
	int nVersion = 0;
	std::ostringstream ostr;
	for(unsigned int i=0; i<vLines.size(); i++) {
		char *pszLine = vLines[i].first;
		unsigned int nLength = vLines[i].second;
		if(nLength > 9 && pszLine[0] == '#' && memcmp(pszLine, "#version ", 9) == 0)
			nVersion = atoi(pszLine+9);
		if(nLength > 11 && pszLine[0] == '#' && memcmp(pszLine, "#include \"", 10) == 0) {
			pszLine += 10; // Skip over the '#include "'
			pszLine[nLength-11] = 0; // Strip off the closing quote (comments and whitespace already stripped, so it should be the last char)
#ifdef _WIN32
			_strlwr(pszLine); // We need to downcase the filename for the map lookup
#endif
			std::map<std::string, ShaderFile>::iterator it = mapShaderFiles.find(pszLine);
			if(it == mapShaderFiles.end()) {
				vk.addWarning(Logger::format("%s: Failed to include %s", getInfo().c_str(), pszLine));
				return false;
			}
			add_dependency(it->second.getInfo());
			ostr << it->second.getContents() << std::endl;
		} else {
			pszLine[nLength] = 0; // Make sure the string is NULL-terminated
			ostr << pszLine << std::endl; // It saves an alloc+copy for this statement
		}
	}

	std::string strPreparedCode = ostr.str();

	// Now attempt to build each technique block found
	for(unsigned int nBlock = 0; nBlock < vTechniqueBlocks.size(); nBlock++) {
		// Parse just enough to get the name for m_mapTechniques
		String::Parser &p = vTechniqueBlocks[nBlock];
		char szToken[256], szName[256];
		p.nextToken(szToken); // Should always be "technique"
		p.nextToken(szName); // Should always be technique name
		p.nextToken(szToken); // Should always be "{"
		p.resetIndex(); // Reset the index for ShaderTechnique::parse()
		if(*szName == '{') {
			vk.addWarning(Logger::format("%s: 'technique' declared without name", getInfo().c_str()));
			continue;
		}
		if(*szToken != '{') {
			vk.addWarning(Logger::format("%s: 'technique' %s declared incorrectly", getInfo().c_str(), szName));
			continue;
		}

		// Now call ShaderTechnique::parse() to finish the job
		ShaderTechnique &technique = mapTechniques[szName];
		VKLogInfo("Parsing technique: %s", szName);
		if(!technique.parse(p, strPreparedCode, nVersion)) {
			VKLogError("Failed to parse technique: %s", szName);
			if(!technique.isValid()) // If this technique has never been compiled successfully, erase it from the map
				mapTechniques.erase(szName);
			continue;
		}
	}

	return true;
}

} // namespace VK
