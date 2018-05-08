#include <stdlib.h>
#include "esUtil.h"

typedef struct {
	GLuint programObject;
	GLint mvpLoc;
	GLfloat *vertices;
	GLuint *indices;
	int numIndices;
	GLfloat angle;
	ESMatrix mvpMatrix;
}UserData;

int Init(ESContext *esContext)
{
	UserData *userData = esContext->userData;
	const char vShaderStr[] =
		"#version 300 es\n"
		"uniform mat4 u_mvpmatrix;\n"
		"layout(location = 0) in vec4 a_position;\n"
		"layout(location = 1) in vec4 a_color;\n"
		"out vec4 v_color;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = u_mvpmatrix*a_position;\n"
		"  v_color = a_color;\n"
		"}\n";
	const char fShaderStr[] =
		"#version 300 es\n"
		"precision mediump float;\n"
		"in vec4 v_color;\n"
		"layout(location = 0) out vec4 outColor;\n"
		"void main()\n"
		"{\n"
		"  outColor = v_color;\n"
		"}\n";

	userData->programObject = esLoadProgram(vShaderStr, fShaderStr);

	userData->mvpLoc = glGetUniformLocation(userData->programObject, "u_mvpmatrix");

	userData->numIndices = esGenCube(0.5, &userData->vertices, NULL, NULL, &userData->indices);

	userData->angle = 60.0f;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	return GL_TRUE;
}

void Update(ESContext *esContext, float deltaTime)
{
	UserData *userData = esContext->userData;
	ESMatrix perspective;
	ESMatrix modeview;
	float aspect;

	userData->angle += deltaTime * 40.0f;
	if (userData->angle >= 360.0f)
		userData->angle -= 360.0f;

	aspect = (GLfloat)esContext->width / (GLfloat)esContext->height;

	esMatrixLoadIdentity(&perspective);
	esPerspective(&perspective, 60.0f, aspect, 1.0f, 20.0f);

	esMatrixLoadIdentity(&modeview);
	esTranslate(&modeview, (userData->angle / 360.0f)*2.0f - 1.0, 0.0, -2.0);

	esRotate(&modeview, userData->angle, 1.0, 0.0, 1.0);

	esMatrixMultiply(&userData->mvpMatrix, &modeview, &perspective);
}

void Draw(ESContext *esContext)
{
	UserData *userData = esContext->userData;

	// Set the viewport
	glViewport(0, 0, esContext->width, esContext->height);

	// Clear the color buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use the program object
	glUseProgram(userData->programObject);

	// Load the vertex position
	glVertexAttribPointer(0, 3, GL_FLOAT,
		GL_FALSE, 3 * sizeof(GLfloat), userData->vertices);

	glEnableVertexAttribArray(0);

	// Set the vertex color to red
	glVertexAttrib4f(1, 1.0f, 0.0f, 0.0f, 1.0f);

	// Load the MVP matrix
	glUniformMatrix4fv(userData->mvpLoc, 1, GL_FALSE, (GLfloat *)&userData->mvpMatrix.m[0][0]);

	// Draw the cube
	glDrawElements(GL_TRIANGLES, userData->numIndices, GL_UNSIGNED_INT, userData->indices);
}

void ShutDown(ESContext *esContext)
{
	UserData *userData = esContext->userData;

	if (userData->vertices != NULL)
	{
		free(userData->vertices);
	}

	if (userData->indices != NULL)
	{
		free(userData->indices);
	}

	// Delete program object
	glDeleteProgram(userData->programObject);
}

int esMain(ESContext *esContext)
{
	esContext->userData = malloc(sizeof(UserData));

	esCreateWindow(esContext, "Simple_VertexShader", 320, 240, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

	if (!Init(esContext))
	{
		return GL_FALSE;
	}

	esRegisterShutdownFunc(esContext, ShutDown);
	esRegisterUpdateFunc(esContext, Update);
	esRegisterDrawFunc(esContext, Draw);

	return GL_TRUE;
}