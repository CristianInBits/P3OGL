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

#define USE_DELTA_TIME

//////////////////////////////////////////////////////////////
// Datos que se almacenan en la memoria de la CPU
//////////////////////////////////////////////////////////////

// Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);

// ===== TAREA 1: Cámara FPS =====
glm::vec3 CoP = glm::vec3(0.0f, 0.0f, 6.0f);
glm::vec3 lookAt = glm::vec3(0.0f, 0.0f, -1.0f);
const glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

float moveSpeed = 0.2f;
float rotationSpeed = 0.05f;

// ===== TAREA 2: Proyección manual =====
const float nearPlane = 0.1f;
const float farPlane = 50.0f;
const float fovY = 60.0f; // grados
const float inv_t30 = 1.0f / tan(glm::radians(fovY / 2.0f)); // 1/tan(30°)

//////////////////////////////////////////////////////////////
// Variables que nos dan acceso a Objetos OpenGL
//////////////////////////////////////////////////////////////
// Handles del shader y programa
unsigned int vshader;
unsigned int fshader;
unsigned int program;

// Variables Uniform
int uModelViewMat;
int uModelViewProjMat;
int uNormalMat;

// Atributos
int inPos;
int inColor;
int inNormal;
int inTexCoord;

// Carga del modelo
unsigned int vao;
unsigned int buffs[5];

// Texturas
unsigned int colorTexId;
unsigned int emiTexId;

// Uniforms de texturas
int uColorTex;
int uEmiTex;

//////////////////////////////////////////////////////////////
// Funciones auxiliares
//////////////////////////////////////////////////////////////

// Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

// Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char* vname, const char* fname);
void initObj();
void destroy();

GLuint loadShader(const char* fileName, GLenum type);
unsigned int loadTex(const char* fileName);

// ===== TAREA 1: Funciones de cámara =====
glm::vec3 rotateY(glm::vec3 v, float angle);
void updateView();


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();

	destroy();

	return 0;
}

//////////////////////////////////////////
// TAREA 1: Funciones de cámara
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
	glClearColor(0.941f, 0.922f, 0.863f, 0.0f);

	glFrontFace(GL_CCW);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);

	// TAREA 2: Proyección manual inicial (aspect ratio = 1.0 para ventana cuadrada)
	proj = glm::mat4(0.0f);
	proj[0][0] = inv_t30;  // aspect = 1.0 inicialmente
	proj[1][1] = inv_t30;
	proj[2][2] = (nearPlane + farPlane) / (nearPlane - farPlane);
	proj[3][2] = 2.0f * farPlane * nearPlane / (nearPlane - farPlane);
	proj[2][3] = -1.0f;

	// TAREA 1: Construir la vista desde la cámara
	updateView();
}

void destroy()
{
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	glDeleteBuffers(5, buffs);
	glDeleteVertexArrays(1, &vao);

	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
}

void initShader(const char* vname, const char* fname)
{
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	glLinkProgram(program);

	int linked;
	glGetProgramiv(program, GL_LINK_STATUS, &linked);
	if (!linked)
	{
		GLint logLen;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);
		char* logString = new char[logLen];
		glGetProgramInfoLog(program, logLen, NULL, logString);
		std::cout << "Error: " << logString << std::endl;
		delete[] logString;
		glDeleteProgram(program);
		program = 0;
		exit(-1);
	}

	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	inPos = 0;
	inColor = 1;
	inNormal = 2;
	inTexCoord = 3;
}

void initObj()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(5, buffs);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[0]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexPos, GL_STATIC_DRAW);
	glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inPos);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[1]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexColor, GL_STATIC_DRAW);
	glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inColor);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[2]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexNormal, GL_STATIC_DRAW);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inNormal);

	glBindBuffer(GL_ARRAY_BUFFER, buffs[3]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 2,
		cubeVertexTexCoord, GL_STATIC_DRAW);
	glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inTexCoord);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffs[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex * sizeof(unsigned int) * 3,
		cubeTriangleIndex, GL_STATIC_DRAW);

	model = glm::mat4(1.0f);

	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char* fileName, GLenum type)
{
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source,
		(const GLint*)&fileLen);
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
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);

	delete[] map;

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);

	return texId;
}

//////////////////////////////////////////
// Callbacks
//////////////////////////////////////////

void renderFunc()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	if (uModelViewMat != -1)
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	if (uModelViewProjMat != -1)
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	if (uNormalMat != -1)
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));

	if (uColorTex != -1)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorTexId);
		glUniform1i(uColorTex, 0);
	}

	if (uEmiTex != -1)
	{
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, emiTexId);
		glUniform1i(uEmiTex, 1);
	}

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);

	glutSwapBuffers();
}

// ===== TAREA 2: resizeFunc con proyección manual =====
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
	model = glm::mat4(1.0f);
	static float angle = 0.0f;

#ifdef USE_DELTA_TIME
	static auto lastTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> deltaTime = currentTime - lastTime;
	lastTime = currentTime;

	const float speed = 1.0f;
	angle += speed * deltaTime.count();

	if (angle > 2.0f * 3.141592f)
		angle -= 2.0f * 3.141592f;
#else
	angle = (angle > 3.141592f * 2.0f) ? 0.0f : angle + 0.01f;
#endif

	model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));

	glutPostRedisplay();
}

// ===== TAREA 1: keyboardFunc con cámara FPS =====
void keyboardFunc(unsigned char key, int x, int y)
{
	glm::vec3 forward = glm::normalize(lookAt);
	glm::vec3 right = glm::normalize(glm::cross(forward, UP));
	glm::vec3 up = glm::cross(right, forward);

	switch (key)
	{
	case 'w': CoP += forward * moveSpeed; break;
	case 's': CoP -= forward * moveSpeed; break;
	case 'd': CoP += right * moveSpeed; break;
	case 'a': CoP -= right * moveSpeed; break;
	case 'u': CoP += up * moveSpeed; break;
	case 'i': CoP -= up * moveSpeed; break;
	case 'q': lookAt = rotateY(lookAt, rotationSpeed); break;
	case 'e': lookAt = rotateY(lookAt, -rotationSpeed); break;
	case 27:  exit(0); break;
	}

	updateView();

	glutPostRedisplay();
}

void mouseFunc(int button, int state, int x, int y) {}