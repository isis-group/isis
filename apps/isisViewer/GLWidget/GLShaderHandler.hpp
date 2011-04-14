#ifndef GLSHADER_HPP
#define GLSHADER_HPP

#define GL_GLEXT_PROTOTYPES

#include <GL/gl.h>
#include <iostream>
#include <map>
#include "common.hpp"

namespace isis {
namespace viewer {

class GLShader
{
public:
	enum ShaderType { fragment, vertex };
	void setSourceCode( const std::string &source ) { m_SourceCode = source; }
	void setShaderID ( const GLuint id ) { m_ShaderID = id; }
	void setShaderType ( const ShaderType &type ) { m_ShaderType = type; }
	
	GLuint getShaderID() const { return m_ShaderID; }
private:
	std::string m_SourceCode;
	GLuint m_ShaderID;
	ShaderType m_ShaderType;
};

	
class GLShaderHandler
{
public:
	GLShaderHandler();
	typedef std::map<std::string, GLShader> ShaderMapType;
	
	ShaderMapType getShaderMap() const { return m_ShaderMap; }
	
	void addShader( const std::string &name, const std::string &source, const GLShader::ShaderType &shaderType);
	void addShader( const std::string &name, const GLShader &shader );
	void removeShader( const std::string &name );
	bool isEnabled() const { return m_isEnabled; }
	void setEnabled( bool enable );
	void createContext () { 
		m_ProgramID = glCreateProgram(); 
		glLinkProgram( m_ProgramID );
		m_Context = true;
	}
	template <typename TYPE>
	bool addVariable( const std::string &name, TYPE var ) 
	{
		if(m_Context)
		{
			GLint _var = glGetUniformLocation(m_ProgramID, name.c_str());
			glUniform1f(_var, var); 
			return true;
		} else {
			return false;
		}
		
		
	}
	
private:
	ShaderMapType m_ShaderMap;
	GLuint m_ProgramID;
	bool m_isEnabled;
	bool m_Context;
};
	
	
}} //end namespace




#endif