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

//Matrices
glm::mat4	proj = glm::mat4(1.0f);
glm::mat4	view = glm::mat4(1.0f);
glm::mat4	model = glm::mat4(1.0f);


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
//!!Por implementar

//Declaración de CB
void renderFunc();
void resizeFunc(int width, int height);
void idleFunc();
void keyboardFunc(unsigned char key, int x, int y);
void mouseFunc(int button, int state, int x, int y);

//Funciones de inicialización y destrucción
void initContext(int argc, char** argv);
void initOGL();
void initShader(const char *vname, const char *fname);
void initObj();
void destroy();


//Carga el shader indicado, devuele el ID del shader
//!Por implementar
GLuint loadShader(const char *fileName, GLenum type);

//Crea una textura, la configura, la sube a OpenGL, 
//y devuelve el identificador de la textura 
//!!Por implementar
unsigned int loadTex(const char *fileName);


int main(int argc, char** argv)
{
	std::locale::global(std::locale("spanish"));// acentos ;)

	initContext(argc, argv);
	initOGL();
	initShader("../shaders_P3/shader.v1.vert", "../shaders_P3/shader.v1.frag");
	initObj();

	glutMainLoop();

	destroy();

	return 0;
}
	
//////////////////////////////////////////
// Funciones auxiliares 
void initContext(int argc, char** argv)
{
	// 1 -- Inicializar la librería
	glutInit(&argc, argv);

	// 2 -- Pedir versión y perfil
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	// 3 -- Configurar el framebuffer por defecto
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	// 4 -- Tamaño y posición de la ventana
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);

	// 5 -- Crear la ventana (y el contexto)
	glutCreateWindow("Práctica OGL");

	// 6 -- Inicializar extensiones
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		exit(-1);
	}
	const GLubyte* oglVersion = glGetString(GL_VERSION);
	std::cout << "This system supports OpenGL Version: " << oglVersion << std::endl;

	// 7 -- Registrar callbacks
	glutReshapeFunc(resizeFunc);
	glutDisplayFunc(renderFunc);
	glutIdleFunc(idleFunc);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
}
void initOGL()
{
	// 1 -- Activar el test de profundidad
	glEnable(GL_DEPTH_TEST);

	// 2 -- Establecer el color de fondo
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

	// 3 -- Cara frontal: vértices en orden antihorario
	glFrontFace(GL_CCW);

	// 4 -- Modo de relleo: sólido
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// 5 -- Activar el backface culling
	glEnable(GL_CULL_FACE);

	// 6 -- Matriz de proyección en perspectiva
	proj = glm::perspective(glm::radians(60.0f), 1.0f, 0.1f, 50.0f);

	// 7 -- Matriz de vista (cámara)
	view = glm::mat4(1.0f);
	view[3].z = -6;
}

void destroy()
{
	// Shaders (Paso 3)
	glDetachShader(program, vshader);
	glDetachShader(program, fshader);
	glDeleteShader(vshader);
	glDeleteShader(fshader);
	glDeleteProgram(program);

	// Geometría (Paso 4)
	glDeleteBuffers(5, buffs);
	glDeleteVertexArrays(1, &vao);

	// Liberar texturas
	glDeleteTextures(1, &colorTexId);
	glDeleteTextures(1, &emiTexId);
}

void initShader(const char* vname, const char* fname)
{
	// Compilar
	vshader = loadShader(vname, GL_VERTEX_SHADER);
	fshader = loadShader(fname, GL_FRAGMENT_SHADER);

	// Crear programa y adjuntar shaders
	program = glCreateProgram();
	glAttachShader(program, vshader);
	glAttachShader(program, fshader);

	// Ya NO necesitamos glBindAttribLocation:
	// los layouts en el shader hacen ese trabajo.

	// Enlazar
	glLinkProgram(program);

	// Comprobar errores de enlazado
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

	// Obtener locations de uniforms (sigue siendo necesario)
	uNormalMat = glGetUniformLocation(program, "normal");
	uModelViewMat = glGetUniformLocation(program, "modelView");
	uModelViewProjMat = glGetUniformLocation(program, "modelViewProj");

	uColorTex = glGetUniformLocation(program, "colorTex");
	uEmiTex = glGetUniformLocation(program, "emiTex");

	// Ya NO necesitamos glGetAttribLocation:
	// sabemos que inPos=0, inColor=1, inNormal=2, inTexCoord=3
	// porque los declaramos nosotros en el shader.
	inPos = 0;
	inColor = 1;
	inNormal = 2;
	inTexCoord = 3;
}

void initObj()
{
	// Crear y activar VAO
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(5, buffs);

	// buffs[0] -> posiciones -> atributo inPos (location 0)
	glBindBuffer(GL_ARRAY_BUFFER, buffs[0]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexPos,
		GL_STATIC_DRAW
	);
	glVertexAttribPointer(inPos, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inPos);

	// buffs[1] -> colores -> atributo inColor (location 1)
	glBindBuffer(GL_ARRAY_BUFFER, buffs[1]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexColor,
		GL_STATIC_DRAW
	);
	glVertexAttribPointer(inColor, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inColor);

	// buffs[2] -> normales -> atributo inNormal (location 2)
	glBindBuffer(GL_ARRAY_BUFFER, buffs[2]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 3,
		cubeVertexNormal,
		GL_STATIC_DRAW
		);
	glVertexAttribPointer(inNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inNormal);

	// buffs[3] -> coord. textura -> atribut
	glBindBuffer(GL_ARRAY_BUFFER, buffs[3]);
	glBufferData(GL_ARRAY_BUFFER,
		cubeNVertex * sizeof(float) * 2,
		cubeVertexTexCoord,
		GL_STATIC_DRAW
	);
	glVertexAttribPointer(inTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(inTexCoord);

	// buffs[4] -> índices de triángulos
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffs[4]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		cubeNTriangleIndex * sizeof(unsigned int) * 3,
		cubeTriangleIndex,
		GL_STATIC_DRAW
	);

	// Inicializar la matriz model
	model = glm::mat4(1.0f);

	// Cargar las texturas
	colorTexId = loadTex("../img/color2.png");
	emiTexId = loadTex("../img/emissive.png");
}

GLuint loadShader(const char* fileName, GLenum type)
{
	// 1. Leer el fichero GLSL a memoria
	unsigned int fileLen;
	char* source = loadStringFromFile(fileName, fileLen);

	// 2. Crear el shader object y compilar
	GLuint shader;
	shader = glCreateShader(type);
	glShaderSource(shader, 1,
		(const GLchar**)&source,
		(const GLint*)&fileLen);
	glCompileShader(shader);
	delete[] source;

	// 3. Comprobar errores de compilación
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

	// 4. Devolver el handle del shader compilado
	return shader;
}

unsigned int loadTex(const char* fileName)
{
	// Cargar imagen a CPU
	unsigned char* map;
	unsigned int w, h;
	map = loadTexture(fileName, w, h);
	if (!map)
	{
		std::cout << "Error cargando el fichero: "
			<< fileName << std::endl;
		exit(-1);
	}

	// Crear textura en GPU y subir datos
	unsigned int texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)map);

	// Liberar memoria CPU
	delete[] map;

	// Generar mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Configurar filtrado y wrapping
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP);

	return texId;
}

void renderFunc()
{
	// 1. Limpiar buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 2. Activar programa
	glUseProgram(program);

	// 3. Calcular matrices
	glm::mat4 modelView = view * model;
	glm::mat4 modelViewProj = proj * view * model;
	glm::mat4 normal = glm::transpose(glm::inverse(modelView));

	// 4. Subir matrices como uniforms
	if (uModelViewMat != -1) {
		glUniformMatrix4fv(uModelViewMat, 1, GL_FALSE,
			&(modelView[0][0]));
	}
	if (uModelViewProjMat != -1) {
		glUniformMatrix4fv(uModelViewProjMat, 1, GL_FALSE,
			&(modelViewProj[0][0]));
	}
	if (uNormalMat != -1) {
		glUniformMatrix4fv(uNormalMat, 1, GL_FALSE,
			&(normal[0][0]));
	}

	// Activar y bindear texturas
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

	// 5. Activar VAO y dibujar
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, cubeNTriangleIndex * 3,
		GL_UNSIGNED_INT, (void*)0);

	// 6. Presentar frame
	glutSwapBuffers();
}

void resizeFunc(int width, int height)
{
	if (height == 0)
	{
		height = 1;
	}

	glViewport(0, 0, width, height);

	float aspectRatio = (float)width / (float)height;
	proj = glm::perspective(glm::radians(60.0f), aspectRatio, 0.1f, 50.0f);

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

	model = glm::rotate(model, angle, glm::normalize(glm::vec3(0.0f, 1.0f, 0.0f)));

	glutPostRedisplay();
}

void keyboardFunc(unsigned char key, int x, int y){}
void mouseFunc(int button, int state, int x, int y){}
