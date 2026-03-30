#include "Shader.h"

#include <stdexcept>

namespace engine
{
	static GLuint compileShader(GLenum type, const std::string& src)
	{
		GLuint shader = glCreateShader(type);
		const char* srcPtr = src.data();
		glShaderSource(shader, 1, &srcPtr, nullptr);
		glCompileShader(shader);

		GLint ok;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok)
		{
			char infoLog[512];
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			glDeleteShader(shader);
			throw std::runtime_error(std::string("Shader compilation failed: ") + infoLog);
		}
	}

	static GLuint linkProgram(GLuint vertShader, GLuint fragShader)
	{
		GLuint program = glCreateProgram();
		glAttachShader(program, vertShader);
		glAttachShader(program, fragShader);
		glLinkProgram(program);

		GLint ok;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &ok);
		if (!ok)
		{
			char infoLog[512];
			glGetProgramInfoLog(program, 512, nullptr, infoLog);
			glDeleteProgram(vertShader);
			glDeleteProgram(fragShader);
			throw std::runtime_error(std::string("Program linking failed: ") + infoLog);
		}

		glDeleteProgram(vertShader);
		glDeleteProgram(fragShader);
		return program;
	}

	Shader::Shader(const std::string& vertSrc, const std::string& fragSrc)
	{
		GLuint vertShader = compileShader(GL_VERTEX_SHADER, vertSrc);
		GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, fragSrc);
		_pid = linkProgram(vertShader, fragShader);
	}

	void Shader::bind() const
	{
		glUseProgram(_pid);
	}

	void Shader::unbind() const
	{
		glUseProgram(0);
	}

	GLint Shader::getUniform(const std::string& name) const
	{
		// Get previously cached uniform locations
		auto uniformIt = _uniforms.find(name);
		if (uniformIt != _uniforms.end())
		{
			return uniformIt->second;
		}

		// Get and cache new uniform location
		GLint loc = glGetUniformLocation(_pid, name.data());
		assert(loc != -1, "Uniform not found: " + name);
		_uniforms.emplace(name, loc);
		return loc;
	}

	void Shader::setInt(const std::string& name, int value) const
	{
		glUniform1i(getUniform(name), value);
	}

	void Shader::setFloat(const std::string& name, float value) const
	{
		glUniform1f(getUniform(name), value);
	}

	void Shader::setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(getUniform(name), 1, &value[0]);
	}

	void Shader::setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(getUniform(name), 1, &value[0]);
	}

	void Shader::setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(getUniform(name), 1, &value[0]);
	}

	void Shader::setMat2(const std::string& name, const glm::mat2& value) const
	{
		glUniformMatrix2fv(getUniform(name), 1, GL_FALSE, &value[0][0]);
	}

	void Shader::setMat3(const std::string& name, const glm::mat3& value) const
	{
		glUniformMatrix3fv(getUniform(name), 1, GL_FALSE, &value[0][0]);
	}

	void Shader::setMat4(const std::string& name, const glm::mat4& value) const
	{
		glUniformMatrix4fv(getUniform(name), 1, GL_FALSE, &value[0][0]);
	}
}