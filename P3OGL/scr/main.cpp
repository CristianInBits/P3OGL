#include "BOX.h"
#include "auxiliar.h"

#include <gl/glew.h>
#define SOLVE_FGLUT_WARNING
#include <gl/freeglut.h> 

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <chrono>
#include <algorithm>
#include <vector>
#include <functional>

// Optativa 4: Clases OO
#include "ShaderProgram.h"
#include "Mesh.h"
#include "Material.h"
#include "Light.h"
#include "SceneObject.h"

#define USE_DELTA_TIME

// ============================================================================
// SELECCIÓN DE TAREAS
// ============================================================================
// Tareas 1 y 2 (cámara FPS + proyección manual) siempre activas.
//
// ENABLE_TAREA3    : luz como uniform
// ENABLE_TAREA4    : segundo cubo orbitante
// ENABLE_TAREA5    : dos programas shader
// ENABLE_OPTATIVA1 : múltiples fuentes de luz (shaders v4/v5)
// ENABLE_OPTATIVA2 : geometría asociada a cada luz (shader.light)
// ENABLE_OPTATIVA3 : trayectorias cíclicas de las luces
// ENABLE_OPTATIVA4 : arquitectura OO (este main)
// ============================================================================
#define ENABLE_TAREA3
#define ENABLE_TAREA4
#define ENABLE_TAREA5
#define ENABLE_OPTATIVA1
#define ENABLE_OPTATIVA2
#define ENABLE_OPTATIVA3
#define ENABLE_OPTATIVA4

// Validación de dependencias
#if defined(ENABLE_TAREA5) && !defined(ENABLE_TAREA4)
#error "ENABLE_TAREA5 requiere ENABLE_TAREA4"
#endif
#if defined(ENABLE_TAREA5) && !defined(ENABLE_TAREA3)
#error "ENABLE_TAREA5 requiere ENABLE_TAREA3"
#endif
#if defined(ENABLE_TAREA4) && !defined(ENABLE_TAREA3)
#error "ENABLE_TAREA4 requiere ENABLE_TAREA3"
#endif
#if defined(ENABLE_OPTATIVA1) && !defined(ENABLE_TAREA3)
#error "ENABLE_OPTATIVA1 requiere ENABLE_TAREA3"
#endif
#if defined(ENABLE_OPTATIVA2) && !defined(ENABLE_OPTATIVA1)
#error "ENABLE_OPTATIVA2 requiere ENABLE_OPTATIVA1"
#endif
#if defined(ENABLE_OPTATIVA3) && !defined(ENABLE_OPTATIVA1)
#error "ENABLE_OPTATIVA3 requiere ENABLE_OPTATIVA1"
#endif

// ============================================================================
// Constantes
// ============================================================================
#ifdef ENABLE_OPTATIVA1
const int MAX_LIGHTS = 4;
const int NUM_LIGHTS = 3;
#endif

//////////////////////////////////////////////////////////////
// Objetos OO (Optativa 4)
//////////////////////////////////////////////////////////////

// Geometría compartida
Mesh cubeMesh;

// Material (texturas)
Material cubeMaterial;

// Programas shader
ShaderProgram shaderTextured;  // sp1: con texturas (v5 o v3 o v1)
#ifdef ENABLE_TAREA5
ShaderProgram shaderFlat;      // sp0: sin texturas (v4 o v2)
#endif
#ifdef ENABLE_OPTATIVA2
ShaderProgram shaderLightGeo;  // Shader emisivo para geometría de luces
#endif

// Objetos de la escena
SceneObject cube1;
#ifdef ENABLE_TAREA4
SceneObject cube2;
float orbitAngle = 0.0f;
#endif

// Luces
#ifdef ENABLE_OPTATIVA1
std::vector<Light> lights;
int selectedLight = 0;
#elif defined(ENABLE_TAREA3)
glm::vec3 lightPosWorld = glm::vec3(0.0f, 3.0f, 3.0f);
float lightIntensity = 1.0f;
#endif

//////////////////////////////////////////////////////////////
// Cámara y proyección (no encapsuladas — dependen de GLUT)
//////////////////////////////////////////////////////////////

glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);

// Tarea 1: Cámara FPS
glm::vec3 CoP = glm::vec3(0.0f, 0.0f, 10.0f);
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

float moveSpeed = 0.2f;
float rotationSpeed = 0.05f;

// Tarea 2: Proyección manual
const float nearPlane = 0.1f;
const float farPlane = 50.0f;
const float fovY = 60.0f;
const float inv_t30 = 1.0f / tan(glm::radians(fovY / 2.0f));

//////////////////////////////////////////////////////////////
// Declaraciones
//////////////////////////////////////////////////////////////
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

void initContext(int argc, char** argv);
void initOGL();
void initScene();
void destroy();

glm::vec3 rotateY(glm::vec3 v, float angle);
void updateView();

#ifdef ENABLE_OPTATIVA1
void uploadLights(const ShaderProgram& sp);
#elif defined(ENABLE_TAREA3)
void uploadLight(const ShaderProgram& sp);
#endif

#ifdef ENABLE_OPTATIVA2
void drawLightGeometry();
#endif

// ============================================================================
// FUNCIONES AUXILIARES
// ============================================================================
void printConfig()
{
	std::cout << "========================================" << std::endl;
	std::cout << "  Practica 3 - Programacion en OpenGL" << std::endl;
	std::cout << "========================================" << std::endl;
	std::cout << "  Tarea 1: Camara FPS            [ON]" << std::endl;
	std::cout << "  Tarea 2: Aspect ratio manual    [ON]" << std::endl;
#ifdef ENABLE_TAREA3
	std::cout << "  Tarea 3: Luz como uniform       [ON]" << std::endl;
#else
	std::cout << "  Tarea 3: Luz como uniform       [OFF]" << std::endl;
#endif
#ifdef ENABLE_TAREA4
	std::cout << "  Tarea 4: Segundo objeto         [ON]" << std::endl;
#else
	std::cout << "  Tarea 4: Segundo objeto         [OFF]" << std::endl;
#endif
#ifdef ENABLE_TAREA5
	std::cout << "  Tarea 5: Dos programas shader   [ON]" << std::endl;
#else
	std::cout << "  Tarea 5: Dos programas shader   [OFF]" << std::endl;
#endif
#ifdef ENABLE_OPTATIVA1
	std::cout << "  Opt. 1:  Multiples luces (" << NUM_LIGHTS << ")   [ON]" << std::endl;
#else
	std::cout << "  Opt. 1:  Multiples luces        [OFF]" << std::endl;
#endif
#ifdef ENABLE_OPTATIVA2
	std::cout << "  Opt. 2:  Geometria de luces     [ON]" << std::endl;
#else
	std::cout << "  Opt. 2:  Geometria de luces     [OFF]" << std::endl;
#endif
#ifdef ENABLE_OPTATIVA3
	std::cout << "  Opt. 3:  Trayectorias ciclicas  [ON]" << std::endl;
#else
	std::cout << "  Opt. 3:  Trayectorias ciclicas  [OFF]" << std::endl;
#endif
	std::cout << "  Opt. 4:  Arquitectura OO        [ON]" << std::endl;
	std::cout << "========================================" << std::endl;
}


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));
	printConfig();

	initContext(argc, argv);
	initOGL();
	initScene();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// Tarea 1: Funciones de cámara
//////////////////////////////////////////

glm::vec3 rotateY(glm::vec3 v, float angle)
{
	glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
	return glm::vec3(rot * glm::vec4(v, 0.0f));
}

void updateView()
{
	glm::vec3 k = glm::normalize(-lookAt);
	glm::vec3 i = glm::normalize(glm::cross(UP, k));
	glm::vec3 j = glm::cross(k, i);

	glm::mat4 M = glm::mat4(1.0f);
	M[0] = glm::vec4(i, 0.0f);
	M[1] = glm::vec4(j, 0.0f);
	M[2] = glm::vec4(k, 0.0f);
	M[3] = glm::vec4(CoP, 1.0f);

	view = glm::inverse(M);
}

//////////////////////////////////////////
// Subida de luces
//////////////////////////////////////////

#ifdef ENABLE_OPTATIVA1
void uploadLights(const ShaderProgram& sp)
{
	glm::vec3 lightPosEye[MAX_LIGHTS];
	glm::vec3 lId[MAX_LIGHTS];
	glm::vec3 lIs[MAX_LIGHTS];

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		lightPosEye[i] = lights[i].getPosEye(view);
		lId[i] = lights[i].diffuseColor;
		lIs[i] = lights[i].specularColor;
	}

	if (sp.uNumLights != -1) glUniform1i(sp.uNumLights, NUM_LIGHTS);
	if (sp.uLightPos != -1)  glUniform3fv(sp.uLightPos, NUM_LIGHTS, &lightPosEye[0][0]);
	if (sp.uLightId != -1)   glUniform3fv(sp.uLightId, NUM_LIGHTS, &lId[0][0]);
	if (sp.uLightIs != -1)   glUniform3fv(sp.uLightIs, NUM_LIGHTS, &lIs[0][0]);
	if (sp.uIa != -1)        glUniform3f(sp.uIa, 0.3f, 0.3f, 0.3f);
}
#elif defined(ENABLE_TAREA3)
void uploadLight(const ShaderProgram& sp)
{
	glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));
	if (sp.uLightPos != -1) glUniform3fv(sp.uLightPos, 1, &lightPosEye[0]);
	if (sp.uIa != -1) glUniform3f(sp.uIa, 0.3f, 0.3f, 0.3f);
	if (sp.uId != -1) glUniform3f(sp.uId, lightIntensity, lightIntensity, lightIntensity);
	if (sp.uIs != -1) glUniform3f(sp.uIs, lightIntensity, lightIntensity, lightIntensity);
}
#endif

//////////////////////////////////////////
// Optativa 2: Geometría de luces
//////////////////////////////////////////

#ifdef ENABLE_OPTATIVA2
void drawLightGeometry()
{
	shaderLightGeo.use();

	const float lightScale = 0.15f;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lights[i].posWorld);
		lightModel = glm::scale(lightModel, glm::vec3(lightScale));

		glm::mat4 modelViewProj = proj * view * lightModel;

		if (shaderLightGeo.uModelViewProjMat != -1)
			glUniformMatrix4fv(shaderLightGeo.uModelViewProjMat, 1, GL_FALSE,
				&(modelViewProj[0][0]));

		if (shaderLightGeo.uLightColor != -1)
			glUniform3fv(shaderLightGeo.uLightColor, 1, &lights[i].diffuseColor[0]);

		cubeMesh.draw();
	}
}
#endif

//////////////////////////////////////////
// Funciones de inicialización
//////////////////////////////////////////

void initContext(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Práctica OGL");

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}

void initOGL()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.471f, 0.627f, 0.824f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	proj = glm::mat4(0.0f);
	proj[0][0] = inv_t30;
	proj[1][1] = inv_t30;
	proj[2][2] = (nearPlane + farPlane) / (nearPlane - farPlane);
	proj[3][2] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
	proj[2][3] = -1.0f;

	updateView();
}

// Optativa 4: initScene reemplaza a initShader + initObj
// Inicializa shaders, geometría, materiales, luces y objetos usando las clases OO
void initScene()
{
	// --- Shaders ---
#ifdef ENABLE_OPTATIVA1
	shaderTextured.init("../shaders_P3/shader.v5.vert", "../shaders_P3/shader.v5.frag");
#elif defined(ENABLE_TAREA3)
	shaderTextured.init("../shaders_P3/shader.v3.vert", "../shaders_P3/shader.v3.frag");
#else
	shaderTextured.init("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
#endif

#ifdef ENABLE_TAREA5
#ifdef ENABLE_OPTATIVA1
	shaderFlat.init("../shaders_P3/shader.v4.vert", "../shaders_P3/shader.v4.frag");
#else
	shaderFlat.init("../shaders_P3/shader.v2.vert", "../shaders_P3/shader.v2.frag");
#endif
#endif

#ifdef ENABLE_OPTATIVA2
	shaderLightGeo.init("../shaders_P3/shader.light.vert", "../shaders_P3/shader.light.frag");
#endif

	// --- Geometría (compartida por todos los objetos) ---
	cubeMesh.load(cubeNVertex, cubeNTriangleIndex,
		cubeVertexPos, cubeVertexColor, cubeVertexNormal,
		cubeVertexTexCoord, cubeTriangleIndex);

	// --- Material (texturas) ---
	cubeMaterial.load("../img/color2.png", "../img/emissive.png");

	// --- Objetos de la escena ---
	// Cubo 1: con texturas
	cube1.mesh = &cubeMesh;
	cube1.material = &cubeMaterial;
	cube1.shader = &shaderTextured;
	cube1.model = glm::mat4(1.0f);

#ifdef ENABLE_TAREA4
	// Cubo 2: sin texturas (Tarea 5) o con texturas (Tarea 4 sola)
	cube2.mesh = &cubeMesh;
#ifdef ENABLE_TAREA5
	cube2.material = nullptr;   // Sin texturas
	cube2.shader = &shaderFlat;
#else
	cube2.material = &cubeMaterial;
	cube2.shader = &shaderTextured;
#endif
	cube2.model = glm::mat4(1.0f);
#endif

	// --- Luces ---
#ifdef ENABLE_OPTATIVA1
	lights.resize(NUM_LIGHTS);

	lights[0] = Light(
		glm::vec3(0.0f, 3.0f, 3.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f));

	lights[1] = Light(
		glm::vec3(-4.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.2f, 0.2f),
		glm::vec3(1.0f, 0.2f, 0.2f));

	lights[2] = Light(
		glm::vec3(4.0f, 1.0f, 0.0f),
		glm::vec3(0.2f, 0.2f, 1.0f),
		glm::vec3(0.2f, 0.2f, 1.0f));

#ifdef ENABLE_OPTATIVA3
	// Trayectorias como lambdas (dos ángulos independientes por luz)
	lights[0].speed = 0.8f;
	lights[0].trajectoryFunc = [](float a, float a2, glm::vec3& p) {
		p.x = 5.0f * cos(a);
		p.z = 5.0f * sin(a);
		p.y = 3.0f;
		};

	lights[1].speed = 1.04f;
	lights[1].trajectoryFunc = [](float a, float a2, glm::vec3& p) {
		p.x = 4.0f * cos(a);
		p.y = 4.0f * sin(a);
		p.z = 0.0f;
		};

	// La azul usa angle para la órbita XZ y angle2 para la oscilación vertical
	// Cada uno se resetea independientemente -> sin saltos
	lights[2].speed = 0.56f;
	lights[2].speed2 = 0.39f;
	lights[2].trajectoryFunc = [](float a, float a2, glm::vec3& p) {
		p.x = 4.0f * cos(a);
		p.z = 4.0f * sin(a);
		p.y = 2.0f * sin(a2);
		};
#endif
#endif
}

void destroy()
{
	shaderTextured.destroy();
#ifdef ENABLE_TAREA5
	shaderFlat.destroy();
#endif
#ifdef ENABLE_OPTATIVA2
	shaderLightGeo.destroy();
#endif

	cubeMesh.destroy();
	cubeMaterial.destroy();
}

//////////////////////////////////////////
// Callbacks
//////////////////////////////////////////

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	cubeMesh.bind();

	// ===== Cubo 1: shaderTextured =====
	shaderTextured.use();

#ifdef ENABLE_OPTATIVA1
	uploadLights(shaderTextured);
#elif defined(ENABLE_TAREA3)
	uploadLight(shaderTextured);
#endif

	cube1.draw(view, proj);

	// ===== Cubo 2 =====
#ifdef ENABLE_TAREA4

#ifdef ENABLE_TAREA5
	shaderFlat.use();
#ifdef ENABLE_OPTATIVA1
	uploadLights(shaderFlat);
#elif defined(ENABLE_TAREA3)
	uploadLight(shaderFlat);
#endif
	cube2.draw(view, proj);
#else
	cube2.draw(view, proj);
#endif

#endif

	// ===== Geometría de luces =====
#ifdef ENABLE_OPTATIVA2
	drawLightGeometry();
#endif

	glutSwapBuffers();
}

void resizeFunc(int width, int height)
{
	if (height == 0) height = 1;

	glViewport(0, 0, width, height);

	float aspectRatio = (float)width / (float)height;
	proj = glm::mat4(0.0f);
	proj[0][0] = inv_t30 / aspectRatio;
	proj[1][1] = inv_t30;
	proj[2][2] = (nearPlane + farPlane) / (nearPlane - farPlane);
	proj[3][2] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
	proj[2][3] = -1.0f;

	glutPostRedisplay();
}

void idleFunc()
{
	static float angle = 0.0f;

#ifdef USE_DELTA_TIME
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	float dt = deltaTime.count();

	const float speed = 1.0f;
	angle += speed * dt;
	if (angle > 2.0f * 3.141592f) angle -= 2.0f * 3.141592f;

#ifdef ENABLE_TAREA4
	orbitAngle += 0.5f * dt;
	if (orbitAngle > 2.0f * 3.141592f) orbitAngle -= 2.0f * 3.141592f;
#endif

#else
	angle = (angle > 3.141592f * 2.0f) ? 0.0f : angle + 0.01f;
#ifdef ENABLE_TAREA4
	orbitAngle = (orbitAngle > 3.141592f * 2.0f) ? 0.0f : orbitAngle + 0.005f;
#endif
#endif

	// Cubo 1: rotación sobre eje diagonal
	cube1.model = glm::mat4(1.0f);
	cube1.model = glm::rotate(cube1.model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

#ifdef ENABLE_TAREA4
	// Cubo 2: órbita + escala + spin
	cube2.model = glm::mat4(1.0f);
	cube2.model = glm::rotate(cube2.model, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	cube2.model = glm::translate(cube2.model, glm::vec3(3.0f, 0.0f, 0.0f));
	cube2.model = glm::scale(cube2.model, glm::vec3(0.5f));
	cube2.model = glm::rotate(cube2.model, orbitAngle * 3.0f, glm::vec3(1.0f, 1.0f, 0.0f));
#endif

#ifdef ENABLE_OPTATIVA3
	// Optativa 3: actualizar trayectorias de luces (OO)
	for (int i = 0; i < NUM_LIGHTS; i++)
		lights[i].update(dt);
#endif

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y)
{
	glm::vec3 forward = glm::normalize(lookAt);
	glm::vec3 right = glm::normalize(glm::cross(forward, UP));
	glm::vec3 up = glm::cross(right, forward);

	switch (key)
	{
		// Cámara (Tarea 1)
	case 'w': CoP += forward * moveSpeed; break;
	case 's': CoP -= forward * moveSpeed; break;
	case 'd': CoP += right * moveSpeed; break;
	case 'a': CoP -= right * moveSpeed; break;
	case 'u': CoP += up * moveSpeed; break;
	case 'i': CoP -= up * moveSpeed; break;
	case 'q': lookAt = rotateY(lookAt, rotationSpeed); break;
	case 'e': lookAt = rotateY(lookAt, -rotationSpeed); break;

#ifdef ENABLE_TAREA3

#ifdef ENABLE_OPTATIVA1
		// Optativa 1: Seleccionar luz activa y moverla
	case '1': selectedLight = 0; std::cout << "Luz seleccionada: 0 (blanca)" << std::endl; break;
	case '2': selectedLight = 1; std::cout << "Luz seleccionada: 1 (roja)" << std::endl; break;
	case '3': selectedLight = 2; std::cout << "Luz seleccionada: 2 (azul)" << std::endl; break;

	case 'j': lights[selectedLight].posWorld.x -= 0.5f; break;
	case 'l': lights[selectedLight].posWorld.x += 0.5f; break;
	case 'o': lights[selectedLight].posWorld.y += 0.5f; break;
	case 'p': lights[selectedLight].posWorld.y -= 0.5f; break;
	case 'k': lights[selectedLight].posWorld.z -= 0.5f; break;
	case ';': lights[selectedLight].posWorld.z += 0.5f; break;
#else
		// Tarea 3: Mover luz única
	case 'j': lightPosWorld.x -= 0.5f; break;
	case 'l': lightPosWorld.x += 0.5f; break;
	case 'o': lightPosWorld.y += 0.5f; break;
	case 'p': lightPosWorld.y -= 0.5f; break;
	case 'k': lightPosWorld.z -= 0.5f; break;
	case ';': lightPosWorld.z += 0.5f; break;

	case '+': lightIntensity = std::min(lightIntensity + 0.1f, 3.0f); break;
	case '-': lightIntensity = std::max(lightIntensity - 0.1f, 0.0f); break;
#endif

#endif

	case 27: exit(0); break;
	}

	updateView();
}

void mouseFunc(int button, int state, int x, int y) {}