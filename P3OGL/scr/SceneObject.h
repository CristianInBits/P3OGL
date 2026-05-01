#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"

// ============================================================================
// Optativa 4: Clase SceneObject
// Agrupa la referencia a un mesh, un material (opcional), un shader,
// y la matriz model. Encapsula el dibujado: activar programa, subir
// matrices, bindear material, dibujar.
// ============================================================================

class SceneObject
{
public:
	Mesh* mesh = nullptr;
	Material* material = nullptr;  // nullptr si el objeto no usa texturas
	ShaderProgram* shader = nullptr;
	glm::mat4 model = glm::mat4(1.0f);

	SceneObject() {}

	SceneObject(Mesh* m, Material* mat, ShaderProgram* sp)
		: mesh(m), material(mat), shader(sp) {}

	// Dibuja el objeto. Asume que el programa ya está activado con use()
	// y que la luz ya se ha subido. Solo sube matrices y material.
	void draw(const glm::mat4& view, const glm::mat4& proj) const
	{
		if (!mesh || !shader) return;

		glm::mat4 modelView = view * model;
		glm::mat4 modelViewProj = proj * modelView;
		glm::mat4 normal = glm::transpose(glm::inverse(modelView));

		if (shader->uModelViewMat != -1)
			glUniformMatrix4fv(shader->uModelViewMat, 1, GL_FALSE,
				&(modelView[0][0]));
		if (shader->uModelViewProjMat != -1)
			glUniformMatrix4fv(shader->uModelViewProjMat, 1, GL_FALSE,
				&(modelViewProj[0][0]));
		if (shader->uNormalMat != -1)
			glUniformMatrix4fv(shader->uNormalMat, 1, GL_FALSE,
				&(normal[0][0]));

		if (material)
			material->bind(*shader);

		mesh->draw();
	}
};
