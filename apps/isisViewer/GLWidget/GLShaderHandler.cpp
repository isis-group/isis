#include "GLShaderHandler.hpp"
#include <boost/foreach.hpp>


namespace isis {
namespace viewer {
	
GLShaderHandler::GLShaderHandler()
	:m_isEnabled(false)
{
	
}	

void GLShaderHandler::addShader(const std::string& name, const std::string& source, const isis::viewer::GLShader::ShaderType& shaderType)
{
	GLShader shader;
	shader.setShaderType( shaderType );
	shader.setSourceCode( source );
	switch( shaderType )
	{
		case GLShader::fragment:
			shader.setShaderID( glCreateShader( GL_FRAGMENT_SHADER ) );
			break;
		case GLShader::vertex:
			shader.setShaderID( glCreateShader( GL_VERTEX_SHADER ) );
			break;
	}
	const GLcharARB * csource  = source.c_str();
	glShaderSource( shader.getShaderID(), 1, &csource, NULL );
	glCompileShader( shader.getShaderID() );
	glAttachShader( m_ProgramID, shader.getShaderID() );
	m_ShaderMap[name] = shader;
}

void GLShaderHandler::setEnabled(bool enable)
{
	if( enable ) {
		BOOST_FOREACH( ShaderMapType::const_reference shaderRef, m_ShaderMap ) 
		glLinkProgram( m_ProgramID );
		glUseProgram( m_ProgramID );
		m_isEnabled = true;
	} else {
		glUseProgram(0);
		m_isEnabled = false;
	}
}

void GLShaderHandler::removeShader(const std::string& name)
{
	glDetachShader( m_ProgramID, m_ShaderMap[name].getShaderID() );
	glDeleteShader( m_ShaderMap[name].getShaderID() );
	m_ShaderMap.erase(name);	
}


void GLShaderHandler::addShader(const std::string &name, const isis::viewer::GLShader& shader)
{
	m_ShaderMap[name] = shader;
}


	
	
}} //end namespace