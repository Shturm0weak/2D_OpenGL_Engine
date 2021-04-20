#include "../pch.h"
#include "Shader.h"

using namespace Doom;

Shader::Shader(const std::string& name, const std::string& filepath) : m_Name(name),m_FilePath(filepath), m_RendererID(0)
{
	ShaderProgramSource source = Parseshader(filepath);
	m_RendererID = CreateShader(source.m_VertexSource, source.m_FragmentSource);
	Logger::Success("has been created!", "Shader", name.c_str());
}

Shader::~Shader()
{
	glDeleteProgram(m_RendererID);
	auto iter = s_Shaders.find(this->m_Name);
	if(iter != s_Shaders.end())
		s_Shaders.erase(iter);
}

Shader* Doom::Shader::Create(const std::string& name, const std::string& path)
{
	auto iter = s_Shaders.find(name);
	if (iter == s_Shaders.end()) 
	{
		s_Shaders.insert(std::make_pair(name, new Shader(name, path)));
		return Get(name);
	}
	else 
	{
#ifdef _DEBUG
		Logger::Warning("has been already existed!", "Shader", name.c_str());
#endif
		return iter->second;
	}
}

Shader * Doom::Shader::Get(const std::string& name, bool showErrors)
{
	auto iter = s_Shaders.find(name);
	if (iter != s_Shaders.end()) return iter->second;
	else if (showErrors)
		Logger::Warning("doesn't exist!", "Shader", name.c_str());
	return nullptr;
}

void Shader::Bind() const 
{
	glUseProgram(m_RendererID);
}

void Shader::UnBind() const 
{
	glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
	glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1iv(const std::string& name, int* value)
{
	glUniform1iv(GetUniformLocation(name),32,value);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) 
{
	glUniform4f(GetUniformLocation(name),v0,v1,v2,v3);
}

void Doom::Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
	glUniform3f(GetUniformLocation(name), v0,v1,v2);
}

void Doom::Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
	glUniform2f(GetUniformLocation(name),v0,v1);
}

void Shader::SetUniform4fv(const std::string& name, const glm::vec4& vec4) 
{
	float vec[4] = { vec4[0],vec4[1],vec4[2],vec4[3] };
	glUniform4fv(GetUniformLocation(name), 1,vec);
}

void Doom::Shader::SetUniform3fv(const std::string& name, const glm::vec3 & vec3)
{
	float vec[3] = { vec3[0],vec3[1],vec3[2]};
	glUniform3fv(GetUniformLocation(name), 1, vec);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) 
{
	glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Doom::Shader::Reload()
{
	glDeleteProgram(m_RendererID);
	ShaderProgramSource source = Parseshader(m_FilePath);
	m_RendererID = CreateShader(source.m_VertexSource, source.m_FragmentSource);
	Logger::Success("has been updated!", "Shader", m_Name.c_str());
}

const char ** Doom::Shader::GetListOfShaders()
{
	if (s_NamesOfShaders != nullptr)
		delete[] s_NamesOfShaders;
	s_NamesOfShaders = new const char*[s_Shaders.size()];
	uint32_t i = 0;
	for (auto mesh = s_Shaders.begin(); mesh != s_Shaders.end(); mesh++)
	{
		s_NamesOfShaders[i] = mesh->first.c_str();
		i++;
	}
	return s_NamesOfShaders;
}

void Shader::SetUniform2fv(const std::string& name, const glm::vec2& vec2)
{
	float vec[2] = { vec2[0],vec2[1] };
	glUniform2fv(GetUniformLocation(name),1,vec);
}

void Shader::SetUniform1f(const std::string& name, float v0) 
{
	glUniform1fv(GetUniformLocation(name), 1, &v0);
}

int Shader::GetUniformLocation(const std::string& name) 
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];
	int location = glGetUniformLocation(m_RendererID, name.c_str());
	if (location == -1)
		Logger::Warning("doesn't exist!", "Uniform", name.c_str());
	m_UniformLocationCache[name] = location;
	return location;
}

ShaderProgramSource Shader::Parseshader(const std::string& filepath)
{
	std::ifstream stream(filepath);

	enum class shadertype {

		NONE = -1, VERTEX = 0, FRAGMENT = 1

	};

	shadertype shadertp = shadertype::NONE;
	std::string line;
	std::string buf;
	std::stringstream ss[2];
	while (getline(stream, line))
	{
		if (line.find("#shader") != std::string::npos)
		{
			if (line.find("vertex") != std::string::npos) 
				shadertp = shadertype::VERTEX;
			else if (line.find("fragment") != std::string::npos)
				shadertp = shadertype::FRAGMENT;
		}
		else
			ss[(int)shadertp] << line << '\n';
	}
	return { ss[0].str(),ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source) 
{
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length * sizeof(char)];
		glGetShaderInfoLog(id, length, &length, message);
		Logger::Error("failed to compile!", "Shader", m_Name.c_str());
		std::cout << message << std::endl;
		delete[] message;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

unsigned int Shader::CreateShader(const std::string& vertexShader, const std::string& FragmentShader)
{
	unsigned int programm = glCreateProgram();
	unsigned vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned fs = CompileShader(GL_FRAGMENT_SHADER, FragmentShader);

	glAttachShader(programm, vs);
	glAttachShader(programm, fs);
	glLinkProgram(programm);
	glValidateProgram(programm);

	glDeleteShader(vs);
	glDeleteShader(fs);

	return programm;
}