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

#define USE_DELTA_TIME

// ============================================================================
// SELECCIÓN DE TAREAS
// ============================================================================
// Tareas 1 y 2 (cámara FPS + proyección manual) siempre activas.
//
// ENABLE_TAREA3    : luz como uniform (shaders v2/v3)
// ENABLE_TAREA4    : segundo cubo orbitante
// ENABLE_TAREA5    : dos programas shader
// ENABLE_OPTATIVA1 : múltiples fuentes de luz (shaders v4/v5)
// ENABLE_OPTATIVA2 : geometría asociada a cada luz (shader.light)
// ============================================================================
#define ENABLE_TAREA3
#define ENABLE_TAREA4
#define ENABLE_TAREA5
#define ENABLE_OPTATIVA1
#define ENABLE_OPTATIVA2

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

// ============================================================================
// Constantes
// ============================================================================
#ifdef ENABLE_OPTATIVA1
const int MAX_LIGHTS = 4;
const int NUM_LIGHTS = 3;
#endif

//////////////////////////////////////////////////////////////
// Struct para encapsular un programa shader
//////////////////////////////////////////////////////////////
struct ShaderProgram
{
	unsigned int vshader = 0;
	unsigned int fshader = 0;
	unsigned int program = 0;

	int uModelViewMat = -1;
	int uModelViewProjMat = -1;
	int uNormalMat = -1;

	int uColorTex = -1;
	int uEmiTex = -1;

#ifdef ENABLE_OPTATIVA1
	// Optativa 1: arrays de luces
	int uNumLights = -1;
	int uLightPos = -1;  // location de lightPos[0]
	int uLightId = -1;   // location de lightId[0]
	int uLightIs = -1;   // location de lightIs[0]
	int uIa = -1;
#else
	// Tarea 3: luz única
	int uLightPos = -1;
	int uIa = -1;
	int uId = -1;
	int uIs = -1;
#endif

	// Optativa 2: color de la luz para geometría emisiva
	int uLightColor = -1;
};

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

// Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

#ifdef ENABLE_TAREA4
glm::mat4	model2 = glm::mat4(1.0f);
float orbitAngle = 0.0f;
#endif

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

#ifdef ENABLE_TAREA3

#ifdef ENABLE_OPTATIVA1
// Optativa 1: Múltiples luces
glm::vec3 lightPosWorld[MAX_LIGHTS] = {
	glm::vec3(0.0f,  3.0f,  3.0f),   // Luz 0: blanca, arriba-delante
	glm::vec3(-4.0f,  1.0f,  0.0f),   // Luz 1: roja, izquierda
	glm::vec3(4.0f,  1.0f,  0.0f),   // Luz 2: azul, derecha
};
glm::vec3 lightId[MAX_LIGHTS] = {
	glm::vec3(1.0f, 1.0f, 1.0f),      // Blanca
	glm::vec3(1.0f, 0.2f, 0.2f),      // Roja
	glm::vec3(0.2f, 0.2f, 1.0f),      // Azul
};
glm::vec3 lightIs[MAX_LIGHTS] = {
	glm::vec3(1.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 0.2f, 0.2f),
	glm::vec3(0.2f, 0.2f, 1.0f),
};
int selectedLight = 0;
#else
// Tarea 3: Luz única
glm::vec3 lightPosWorld = glm::vec3(0.0f, 3.0f, 3.0f);
float lightIntensity = 1.0f;
#endif

#endif // ENABLE_TAREA3

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////

ShaderProgram sp1;
#ifdef ENABLE_TAREA5
ShaderProgram sp0;
#endif
#ifdef ENABLE_OPTATIVA2
ShaderProgram spLight;  // Shader solo emisivo para geometría de luces
#endif

int inPos = 0;
int inColor = 1;
int inNormal = 2;
int inTexCoord = 3;

unsigned int vao;
unsigned int buffs[5];

unsigned int colorTexId;
unsigned int emiTexId;

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
void initShaderProgram(ShaderProgram& sp, const char* vname, const char* fname);
void destroyShaderProgram(ShaderProgram& sp);
void initObj();
void destroy();

GLuint loadShader(const char* fileName, GLenum type);
unsigned int loadTex(const char* fileName);

glm::vec3 rotateY(glm::vec3 v, float angle);
void updateView();
void drawObject(const ShaderProgram& sp, const glm::mat4& objModel);

#ifdef ENABLE_TAREA3
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
	std::cout << "    Luz 0: blanca (0, 3, 3)" << std::endl;
	std::cout << "    Luz 1: roja  (-4, 1, 0)" << std::endl;
	std::cout << "    Luz 2: azul  (4, 1, 0)" << std::endl;
#else
	std::cout << "  Opt. 1:  Multiples luces        [OFF]" << std::endl;
#endif
#ifdef ENABLE_OPTATIVA2
	std::cout << "  Opt. 2:  Geometria de luces     [ON]" << std::endl;
#else
	std::cout << "  Opt. 2:  Geometria de luces     [OFF]" << std::endl;
#endif
	std::cout << "========================================" << std::endl;
}


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));
	printConfig();

	initContext(argc, argv);
	initOGL();

	// Selección de shaders según tareas activas
#ifdef ENABLE_OPTATIVA1
	initShaderProgram(sp1, "../shaders_P3/shader.v5.vert", "../shaders_P3/shader.v5.frag");
#elif defined(ENABLE_TAREA3)
	initShaderProgram(sp1, "../shaders_P3/shader.v3.vert", "../shaders_P3/shader.v3.frag");
#else
	initShaderProgram(sp1, "../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
#endif

#ifdef ENABLE_TAREA5
#ifdef ENABLE_OPTATIVA1
	initShaderProgram(sp0, "../shaders_P3/shader.v4.vert", "../shaders_P3/shader.v4.frag");
#else
	initShaderProgram(sp0, "../shaders_P3/shader.v2.vert", "../shaders_P3/shader.v2.frag");
#endif
#endif

#ifdef ENABLE_OPTATIVA2
	initShaderProgram(spLight,
		"../shaders_P3/shader.light.vert",
		"../shaders_P3/shader.light.frag");
#endif

	initObj();

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
// Funciones auxiliares de render
//////////////////////////////////////////

#ifdef ENABLE_TAREA3
void uploadLight(const ShaderProgram& sp)
{
#ifdef ENABLE_OPTATIVA1
	// Transformar todas las posiciones a eye space
	glm::vec3 lightPosEye[MAX_LIGHTS];
	for (int i = 0; i < NUM_LIGHTS; i++)
		lightPosEye[i] = glm::vec3(view * glm::vec4(lightPosWorld[i], 1.0f));

	if (sp.uNumLights != -1) glUniform1i(sp.uNumLights, NUM_LIGHTS);
	if (sp.uLightPos != -1)  glUniform3fv(sp.uLightPos, NUM_LIGHTS, &lightPosEye[0][0]);
	if (sp.uLightId != -1)   glUniform3fv(sp.uLightId, NUM_LIGHTS, &lightId[0][0]);
	if (sp.uLightIs != -1)   glUniform3fv(sp.uLightIs, NUM_LIGHTS, &lightIs[0][0]);
	if (sp.uIa != -1)        glUniform3f(sp.uIa, 0.3f, 0.3f, 0.3f);
#else
	glm::vec3 lightPosEye = glm::vec3(view * glm::vec4(lightPosWorld, 1.0f));
	if (sp.uLightPos != -1) glUniform3fv(sp.uLightPos, 1, &lightPosEye[0]);
	if (sp.uIa != -1) glUniform3f(sp.uIa, 0.3f, 0.3f, 0.3f);
	if (sp.uId != -1) glUniform3f(sp.uId, lightIntensity, lightIntensity, lightIntensity);
	if (sp.uIs != -1) glUniform3f(sp.uIs, lightIntensity, lightIntensity, lightIntensity);
#endif
}
#endif

void drawObject(const ShaderProgram& sp, const glm::mat4& objModel)
{
	glm::mat4 modelView = view * objModel;
	glm::mat4 modelViewProj = proj * modelView;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (sp.uModelViewMat != -1)
		glUniformMatrix4fv(sp.uModelViewMat, 1, GL_FALSE, &(modelView[0][0]));
	if (sp.uModelViewProjMat != -1)
		glUniformMatrix4fv(sp.uModelViewProjMat, 1, GL_FALSE, &(modelViewProj[0][0]));
	if (sp.uNormalMat != -1)
		glUniformMatrix4fv(sp.uNormalMat, 1, GL_FALSE, &(normal[0][0]));

	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);
}

#ifdef ENABLE_OPTATIVA2
// Optativa 2: Dibuja un cubo pequeño en la posición de cada luz,
// usando solo el color emisivo (sin iluminación). Reutiliza el VAO
// del cubo principal con una model de translate + scale.
void drawLightGeometry()
{
	glUseProgram(spLight.program);

	const float lightScale = 0.15f;

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		// Construir model: posicionar en la luz y escalar
		glm::mat4 lightModel = glm::mat4(1.0f);
		lightModel = glm::translate(lightModel, lightPosWorld[i]);
		lightModel = glm::scale(lightModel, glm::vec3(lightScale));

		// Solo necesitamos modelViewProj (no hay iluminación)
		glm::mat4 modelViewProj = proj * view * lightModel;

		if (spLight.uModelViewProjMat != -1)
			glUniformMatrix4fv(spLight.uModelViewProjMat, 1, GL_FALSE,
				&(modelViewProj[0][0]));

		// Color emisivo = color difuso de la luz
		if (spLight.uLightColor != -1)
			glUniform3fv(spLight.uLightColor, 1, &lightId[i][0]);

		glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
			GL_UNSIGNED_INT, (void*)0);
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

void initShaderProgram(ShaderProgram& sp, const char* vname, const char* fname)
{
	sp.vshader = loadShader(vname, GL_VERTEX_SHADER);
	sp.fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	sp.program = glCreateProgram();
	glAttachShader(sp.program, sp.vshader);
	glAttachShader(sp.program, sp.fshader);

	glLinkProgram(sp.program);

	int linked;
	glGetProgramiv(sp.program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint logLen;
		glGetProgramiv(sp.program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(sp.program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(sp.program);
		sp.program = 0;
		exit(-1);
	}

	// Uniforms de matrices
	sp.uNormalMat = glGetUniformLocation(sp.program, "normal");
	sp.uModelViewMat = glGetUniformLocation(sp.program, "modelView");
	sp.uModelViewProjMat = glGetUniformLocation(sp.program, "modelViewProj");

	// Uniforms de texturas
	sp.uColorTex = glGetUniformLocation(sp.program, "colorTex");
	sp.uEmiTex = glGetUniformLocation(sp.program, "emiTex");

	// Uniforms de luz
#ifdef ENABLE_OPTATIVA1
	sp.uNumLights = glGetUniformLocation(sp.program, "numLights");
	sp.uLightPos = glGetUniformLocation(sp.program, "lightPos");
	sp.uLightId = glGetUniformLocation(sp.program, "lightId");
	sp.uLightIs = glGetUniformLocation(sp.program, "lightIs");
	sp.uIa = glGetUniformLocation(sp.program, "Ia");
#else
	sp.uLightPos = glGetUniformLocation(sp.program, "lpos");
	sp.uIa = glGetUniformLocation(sp.program, "Ia");
	sp.uId = glGetUniformLocation(sp.program, "Id");
	sp.uIs = glGetUniformLocation(sp.program, "Is");
#endif

	// Optativa 2: location del color emisivo (solo relevante para spLight)
	sp.uLightColor = glGetUniformLocation(sp.program, "lightColor");
}

void destroyShaderProgram(ShaderProgram& sp)
{
	glDetachShader(sp.program, sp.vshader);
	glDetachShader(sp.program, sp.fshader);
	glDeleteShader(sp.vshader);
	glDeleteShader(sp.fshader);
	glDeleteProgram(sp.program);
}

void destroy()
{
	destroyShaderProgram(sp1);
#ifdef ENABLE_TAREA5
	destroyShaderProgram(sp0);
#endif
#ifdef ENABLE_OPTATIVA2
	destroyShaderProgram(spLight);
#endif

	glDeleteBuffers(5, buffs);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(5, buffs);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[0]);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inPos);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[1]);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inColor);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[2]);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 3, cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inNormal);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[3]);
	glBufferData(GL_ARRAY_BUFFER, cubeNVertex * sizeof(float) * 2, cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inTexCoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffs[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeNTriangleIndex * sizeof(unsigned int) * 3, cubeTriangleIndex, GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char* fileName, GLenum type)
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
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteShader(shader);
		exit(-1);
	}

	return shader;
}

unsigned int loadTex(const char* fileName)
{
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: " << fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);
	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

	return texId;
}

//////////////////////////////////////////
// Callbacks
//////////////////////////////////////////

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vao);

	// ===== Cubo 1: sp1 =====
	glUseProgram(sp1.program);

#ifdef ENABLE_TAREA3
	uploadLight(sp1);
#endif

	if (sp1.uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(sp1.uColorTex, 0);
	}
	if (sp1.uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(sp1.uEmiTex, 1);
	}

	drawObject(sp1, model);

	// ===== Cubo 2 =====
#ifdef ENABLE_TAREA4

#ifdef ENABLE_TAREA5
	glUseProgram(sp0.program);
	uploadLight(sp0);
	drawObject(sp0, model2);
#else
	drawObject(sp1, model2);
#endif

#endif

#ifdef ENABLE_OPTATIVA2
	// Optativa 2: dibujar geometría de cada luz (cubos emisivos)
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

	model = glm::mat4(1.0f);
	model = glm::rotate(model, angle, glm::vec3(1.0f, 1.0f, 0.0f));

#ifdef ENABLE_TAREA4
	model2 = glm::mat4(1.0f);
	model2 = glm::rotate(model2, orbitAngle, glm::vec3(0.0f, 1.0f, 0.0f));
	model2 = glm::translate(model2, glm::vec3(3.0f, 0.0f, 0.0f));
	model2 = glm::scale(model2, glm::vec3(0.5f));
	model2 = glm::rotate(model2, orbitAngle * 3.0f, glm::vec3(1.0f, 1.0f, 0.0f));
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
		// Optativa 1: Seleccionar luz activa con 1/2/3 y moverla
	case '1': selectedLight = 0; std::cout << "Luz seleccionada: 0 (blanca)" << std::endl; break;
	case '2': selectedLight = 1; std::cout << "Luz seleccionada: 1 (roja)" << std::endl; break;
	case '3': selectedLight = 2; std::cout << "Luz seleccionada: 2 (azul)" << std::endl; break;

	case 'j': lightPosWorld[selectedLight].x -= 0.5f; break;
	case 'l': lightPosWorld[selectedLight].x += 0.5f; break;
	case 'o': lightPosWorld[selectedLight].y += 0.5f; break;
	case 'p': lightPosWorld[selectedLight].y -= 0.5f; break;
	case 'k': lightPosWorld[selectedLight].z -= 0.5f; break;
	case ';': lightPosWorld[selectedLight].z += 0.5f; break;
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

#endif // ENABLE_TAREA3

	case 27: exit(0); break;
	}

	updateView();
}

void mouseFunc(int button, int state, int x, int y) {}