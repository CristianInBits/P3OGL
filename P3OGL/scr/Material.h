#pragma once

#include <gl/glew.h>
#include <iostream>
#include "ShaderProgram.h"

// Función auxiliar de carga de texturas (declarada en auxiliar.h)
extern unsigned char* loadTexture(const char* fileName, unsigned int& w, unsigned int& h);

// ============================================================================
// Optativa 4: Clase Material
// Encapsula las texturas de color y emisiva. Se encarga de cargarlas,
// configurar mipmaps/filtrado, bindearlas a unidades de textura,
// y liberar los recursos.
// ============================================================================

class Material
{
public:
	unsigned int colorTexId = 0;
	unsigned int emiTexId = 0;

	void load(const char* colorPath, const char* emiPath)
	{
		colorTexId = loadTex(colorPath);
		emiTexId = loadTex(emiPath);
	}

	// Bindea las texturas a las unidades 0 y 1, y sube los uniforms del shader
	void bind(const ShaderProgram& sp) const
	{
		if (sp.uColorTex != -1)
		{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, colorTexId);
			glUniform1i(sp.uColorTex, 0);
		}
		if (sp.uEmiTex != -1)
		{
			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, emiTexId);
			glUniform1i(sp.uEmiTex, 1);
		}
	}

	void destroy()
	{
		glDeleteTextures(1, &colorTexId);
		glDeleteTextures(1, &emiTexId);
	}

private:
	unsigned int loadTex(const char* fileName)
	{
		unsigned char* map;
		unsigned int w, h;
		map = loadTexture(fileName, w, h);
		if (!map)
		{
			std::cout << "Error cargando textura: " << fileName << std::endl;
			exit(-1);
		}

		unsigned int texId;
		glGenTextures(1, &texId);
		glBindTexture(GL_TEXTURE_2D, texId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);
		delete[] map;

		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

		return texId;
	}
};
