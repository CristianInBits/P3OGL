#pragma once

#include <gl/glew.h>
#include <glm/glm.hpp>
#include <iostream>

// ============================================================================
// Optativa 4: Clase ShaderProgram
// Encapsula la compilación, enlazado, obtención de locations y destrucción
// de un programa shader. Evolución del struct ShaderProgram anterior.
// ============================================================================

// Función auxiliar de carga (declarada en auxiliar.h, implementada en auxiliar.cpp)
extern char* loadStringFromFile(const char* fileName, unsigned int& fileLen);

class ShaderProgram
{
public:
	unsigned int program = 0;

	// Locations de uniforms
	int uModelViewMat = -1;
	int uModelViewProjMat = -1;
	int uNormalMat = -1;
	int uColorTex = -1;
	int uEmiTex = -1;
	int uLightColor = -1;

	// Locations de luz (múltiples luces)
	int uNumLights = -1;
	int uLightPos = -1;
	int uLightId = -1;
	int uLightIs = -1;
	int uIa = -1;

	void init(const char* vname, const char* fname)
	{
		unsigned int vs = loadAndCompile(vname, GL_VERTEX_SHADER);
		unsigned int fs = loadAndCompile(fname, GL_FRAGMENT_SHADER);

		program = glCreateProgram();
		glAttachShader(program, vs);
		glAttachShader(program, fs);
		glLinkProgram(program);

		int linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		if (!linked)
		{
			GLint logLen;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
			char* logString = new char[logLen];
			glGetProgramInfoLog(program, logLen, NULL, logString);
			std::cout << "Link error: " << logString << std::endl;
			delete[] logString;
			glDeleteProgram(program);
			program = 0;
			exit(-1);
		}

		// Podemos liberar los shaders individuales tras enlazar
		glDetachShader(program, vs);
		glDetachShader(program, fs);
		glDeleteShader(vs);
		glDeleteShader(fs);

		// Obtener todas las locations (las que no existan devuelven -1)
		uModelViewMat     = glGetUniformLocation(program, "modelView");
		uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");
		uNormalMat        = glGetUniformLocation(program, "normal");
		uColorTex         = glGetUniformLocation(program, "colorTex");
		uEmiTex           = glGetUniformLocation(program, "emiTex");
		uLightColor       = glGetUniformLocation(program, "lightColor");
		uNumLights        = glGetUniformLocation(program, "numLights");
		uLightPos         = glGetUniformLocation(program, "lightPos");
		uLightId          = glGetUniformLocation(program, "lightId");
		uLightIs          = glGetUniformLocation(program, "lightIs");
		uIa               = glGetUniformLocation(program, "Ia");
	}

	void use() const
	{
		glUseProgram(program);
	}

	void destroy()
	{
		if (program != 0)
		{
			glDeleteProgram(program);
			program = 0;
		}
	}

private:
	unsigned int loadAndCompile(const char* fileName, GLenum type)
	{
		unsigned int fileLen;
		char* source = loadStringFromFile(fileName, fileLen);

		GLuint shader = glCreateShader(type);
		glShaderSource(shader, 1, (const GLchar**)&source, (const GLint*)&fileLen);
		glCompileShader(shader);
		delete[] source;

		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled)
		{
			GLint logLen;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
			char* logString = new char[logLen];
			glGetShaderInfoLog(shader, logLen, NULL, logString);
			std::cout << "Compile error (" << fileName << "): " << logString << std::endl;
			delete[] logString;
			glDeleteShader(shader);
			exit(-1);
		}

		return shader;
	}
};
