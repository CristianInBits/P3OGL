#pragma once

#include <gl/glew.h>

// ============================================================================
// Optativa 4: Clase Mesh
// Encapsula la creación de VAO y VBOs, la configuración de atributos
// y el dibujado. Los datos se pasan desde fuera (BOX.h u otras fuentes).
// ============================================================================

class Mesh
{
public:
	unsigned int vao = 0;
	unsigned int buffs[5] = { 0, 0, 0, 0, 0 };
	int numTriangleIndex = 0;

	// Carga la geometría a la GPU. Recibe punteros a los datos y sus tamaños.
	// Asume layout(location): pos=0, color=1, normal=2, texCoord=3.
	void load(int nVertex, int nTriIndex,
		const float* positions,
		const float* colors,
		const float* normals,
		const float* texCoords,
		const unsigned int* indices)
	{
		numTriangleIndex = nTriIndex;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(5, buffs);

		// Posiciones → location 0, 3 componentes
		glBindBuffer(GL_ARRAY_BUFFER, buffs[0]);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3,
			positions, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		// Colores → location 1, 3 componentes
		glBindBuffer(GL_ARRAY_BUFFER, buffs[1]);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3,
			colors, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		// Normales → location 2, 3 componentes
		glBindBuffer(GL_ARRAY_BUFFER, buffs[2]);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 3,
			normals, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		// Coordenadas de textura → location 3, 2 componentes
		glBindBuffer(GL_ARRAY_BUFFER, buffs[3]);
		glBufferData(GL_ARRAY_BUFFER, nVertex * sizeof(float) * 2,
			texCoords, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		// Índices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffs[4]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			nTriIndex * sizeof(unsigned int) * 3,
			indices, GL_STATIC_DRAW);
	}

	void bind() const
	{
		glBindVertexArray(vao);
	}

	void draw() const
	{
		glDrawElements(GL_TRIANGLES, numTriangleIndex * 3,
			GL_UNSIGNED_INT, (void*)0);
	}

	void destroy()
	{
		glDeleteBuffers(5, buffs);
		glDeleteVertexArrays(1, &vao);
	}
};
