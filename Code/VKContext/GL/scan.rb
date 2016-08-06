defines = {}
define = nil
level = 0
IO.foreach("gl3.h") {|line|
	if define
		case line
			when /^#if/ then level += 1
			when /^#endif/
				if level > 0
					level -= 1
				else
					define = nil
				end
			when /^\/\* (ARB_[A-Za-z0-9_]+)/
				defines[define] ||= {}
				defines[define]["includes"] ||= []
				defines[define]["includes"] << $1.strip
			when /GLAPI (.+) APIENTRY (gl.+)\((.+)\);/
				defines[define] ||= {}
				defines[define][$2.strip] = [$1.strip, $3.strip]
			when /typedef (.+) \(APIENTRYP (.+)\) \((.+)\);/
				defines[define] ||= {}
				defines[define][$2.strip] = [$1.strip, $3.strip]
		end
	else
		define = $1.strip if line =~ /#ifndef (GL_(VER|ARB).+)/
	end
}

defines.sort.each {|pair|
	next unless pair[0].index("_VERSION_")
	puts "#ifdef #{pair[0]}"
	pair[1].each {|k, v|
		next unless k.index("gl") == 0 and pair[1].has_key?("PFN#{k.upcase}PROC")
		puts "\tPFN#{k.upcase}PROC m_p#{k[2..-1]};"
	}
	pair[1]["includes"].each {|inc|
		i = defines["GL_#{inc}"]
		next unless i
		puts "\t// GL_#{inc}" if defines.has_key?("GL_#{inc}")
		i.each {|k, v|
			next unless k.index("gl") == 0 and i.has_key?("PFN#{k.upcase}PROC")
			puts "\tPFN#{k.upcase}PROC m_p#{k[2..-1]};"
		} if defines.has_key?("GL_#{inc}")
	} if pair[1].has_key?("includes")
	puts "#endif"
	puts ""
}

defines.sort.each {|pair|
	next unless pair[0].index("_VERSION_")
	puts "#ifdef #{pair[0]}"
	puts "/// @name #{pair[0]} functions"
	puts "//@{"
	pair[1].each {|k, v|
		next unless k.index("gl") == 0 and pair[1].has_key?("PFN#{k.upcase}PROC")
		name = k[2,1].downcase + k[3..-1]
		args = v[1] == "void" ? [] : v[1].split(',').collect {|x| x.split(' ')[-1].sub('*', '') }
		if v[0] == "void"
			puts "\t#{v[0]} #{name}(#{v[1]}) { PRE_GL_CHECK(); m_p#{k[2..-1]}(#{args.join(', ')}); POST_GL_CHECK(); }"
		else
			puts "\t#{v[0]} #{name}(#{v[1]}) { PRE_GL_CHECK(); #{v[0]} r = m_p#{k[2..-1]}(#{args.join(', ')}); POST_GL_CHECK(); return r; }"
		end
	}
	pair[1]["includes"].each {|inc|
		i = defines["GL_#{inc}"]
		next unless i
		puts "\t// GL_#{inc}" if defines.has_key?("GL_#{inc}")
		i.each {|k, v|
			next unless k.index("gl") == 0 and i.has_key?("PFN#{k.upcase}PROC")
			name = k[2,1].downcase + k[3..-1]
			args = v[1] == "void" ? [] : v[1].split(',').collect {|x| x.split(' ')[-1].sub('*', '') }
			if v[0] == "void"
				puts "\t#{v[0]} #{name}(#{v[1]}) { PRE_GL_CHECK(); m_p#{k[2..-1]}(#{args.join(', ')}); POST_GL_CHECK(); }"
			else
				puts "\t#{v[0]} #{name}(#{v[1]}) { PRE_GL_CHECK(); #{v[0]} r = m_p#{k[2..-1]}(#{args.join(', ')}); POST_GL_CHECK(); return r; }"
			end
		} if defines.has_key?("GL_#{inc}")
	} if pair[1].has_key?("includes")
	puts "//@}"
	puts "#endif"
	puts ""
}

defines.sort.each {|pair|
	next unless pair[0].index("_VERSION_")
	puts "#ifdef #{pair[0]}"
	pair[1].each {|k, v|
		next unless k.index("gl") == 0 and pair[1].has_key?("PFN#{k.upcase}PROC")
		puts "\tm_p#{k[2..-1]} = (PFN#{k.upcase}PROC)getProcAddress(\"#{k}\");"
	}
	pair[1]["includes"].each {|inc|
		i = defines["GL_#{inc}"]
		next unless i
		puts "\t// GL_#{inc}" if defines.has_key?("GL_#{inc}")
		i.each {|k, v|
			next unless k.index("gl") == 0 and i.has_key?("PFN#{k.upcase}PROC")
			puts "\tm_p#{k[2..-1]} = (PFN#{k.upcase}PROC)getProcAddress(\"#{k}\");"
		} if defines.has_key?("GL_#{inc}")
	} if pair[1].has_key?("includes")
	puts "#endif"
	puts ""
}

