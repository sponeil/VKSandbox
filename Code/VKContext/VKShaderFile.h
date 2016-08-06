// VKShaderFile.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKShaderFile_h__
#define __VKShaderFile_h__

#include "VKShaderTechnique.h"

namespace VK {

/// This class encapsulates a shader text file.
/// It keeps track of the file name, last save time, and contents.
/// A ShaderFile can have any filename extension except for .glfx.
/// If the file has a .glfx extension, use FXFile instead.
class ShaderFile {
protected:
	Path m_info;
	std::string m_strContents;

public:
	bool operator==(const Path &info) const { return m_info == info; }
	bool operator!=(const Path &info) const { return !operator==(info); }
	bool load(const Path &info);
	const Path &getInfo() const { return m_info; }
	const std::string &getContents() const { return m_strContents; }
};

/// This class encapsulates a glfx shader effect file.
/// It tracks everything ShaderFile tracks, but it also tracks dependencies
/// and has a parse() method to build a map of shader techniques from a glfx
/// file.
class FXFile : public ShaderFile {
protected:
	Path::List m_lDependencies;

	void clear_dependencies() { m_lDependencies.clear(); }
	void add_dependency(const Path &info) { m_lDependencies.push_back(info); }

public:
	typedef Path::List::iterator iterator; ///< Use to iterate through dependency list
	iterator begin() { return m_lDependencies.begin(); } ///< Use to iterate through dependency list
	iterator end() { return m_lDependencies.end(); } ///< Use to iterate through dependency list

	/// Call after load() to parse a new/updated glfx file.
	/// To avoid loading the same include file several times, parse() assumes
	/// that all normal shader text files have already been loaded into memory.
	/// @param[in] vk A file is not necessarily tied to a context, so you must pass one in
	/// @param[in] mapShaderFiles A map of loaded shader text files that can be included by this fx file
	/// @param[out] mapTechniques Parsed techniques are added to this map
	bool parse(Context &vk, std::map<std::string, ShaderFile> &mapShaderFiles, std::map<std::string, ShaderTechnique> &mapTechniques);
};

} // namespace VK

#endif // __VKShaderFile_h__
