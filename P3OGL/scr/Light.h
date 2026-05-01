#pragma once

#include <glm/glm.hpp>
#include <functional>

// ============================================================================
// Optativa 4: Clase Light
// Encapsula una fuente de luz: posición en el mundo, colores difuso/especular,
// y opcionalmente una función de trayectoria que actualiza la posición.
//
// Soporta DOS ángulos independientes (angle y angle2) para trayectorias
// que combinan dos frecuencias distintas (ej: órbita XZ + oscilación vertical).
// Cada ángulo tiene su propia velocidad y su propio reset → sin saltos.
// ============================================================================

class Light
{
public:
	glm::vec3 posWorld;
	glm::vec3 diffuseColor;
	glm::vec3 specularColor;

	// Función de trayectoria: recibe ambos ángulos y escribe la posición.
	// Si es nullptr, la luz es estática (movible por teclado).
	std::function<void(float angle, float angle2, glm::vec3& pos)> trajectoryFunc = nullptr;

	float angle = 0.0f;
	float speed = 0.0f;    // rad/s para el ángulo principal

	float angle2 = 0.0f;
	float speed2 = 0.0f;   // rad/s para el segundo ángulo (0 si no se usa)

	Light() : posWorld(0.0f), diffuseColor(1.0f), specularColor(1.0f) {}

	Light(glm::vec3 pos, glm::vec3 id, glm::vec3 is)
		: posWorld(pos), diffuseColor(id), specularColor(is) {}

	// Actualiza la posición según la trayectoria (si tiene una)
	void update(float dt)
	{
		if (trajectoryFunc)
		{
			angle += speed * dt;
			if (angle > 2.0f * 3.141592f)
				angle -= 2.0f * 3.141592f;

			angle2 += speed2 * dt;
			if (angle2 > 2.0f * 3.141592f)
				angle2 -= 2.0f * 3.141592f;

			trajectoryFunc(angle, angle2, posWorld);
		}
	}

	// Transforma la posición del mundo a eye space
	glm::vec3 getPosEye(const glm::mat4& view) const
	{
		return glm::vec3(view * glm::vec4(posWorld, 1.0f));
	}
};
