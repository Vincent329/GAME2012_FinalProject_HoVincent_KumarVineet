﻿//***************************************************************************
// GAME2012_A4_HoVincent.cpp by Vincent Ho (C) 2020 All Rights Reserved.
//
// Assignment 4 submission.
//
// Description:
// Lighting Example where a point light source is overhead a plane
// use WASD to move camera, and R and F to go up and down. Click mouse and drag to orient camera rotation
// IJKL will move the camera forward, left, down, and right respectively
//
// Repo Change check - is repo pushing correctly? - Vineet
//*****************************************************************************


using namespace std;

#include <cstdlib>
#include <ctime>
#include "vgl.h"
#include "LoadShaders.h"
#include "Light.h"
#include "Shape.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define FPS 60
#define MOVESPEED 0.1f
#define TURNSPEED 0.05f
#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,1,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)


enum keyMasks {
	KEY_FORWARD =  0b00000001,		// 0x01 or 1 or 01
	KEY_BACKWARD = 0b00000010,		// 0x02 or 2 or 02
	KEY_LEFT = 0b00000100,		
	KEY_RIGHT = 0b00001000,
	KEY_UP = 0b00010000,
	KEY_DOWN = 0b00100000,
	KEY_MOUSECLICKED = 0b01000000
	// Any other keys you want to add.
};

// IDs.
GLuint vao, ibo, points_vbo, colors_vbo, uv_vbo, normals_vbo, modelID, viewID, projID;
GLuint program;

// Matrices.
glm::mat4 View, Projection;

// Our bitflags. 1 byte for up to 8 keys.
unsigned char keys = 0; // Initialized to 0 or 0b00000000.

// Camera and transform variables.
float scale = 1.0f, angle = 0.0f;
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function
GLfloat pitch, yaw;
int lastX, lastY;

// Texture variables.
GLuint alexTx, blankTx, brickTx, hedgeTx, groundTx, castleTx, doorTx, roofTx;
GLint width, height, bitDepth;

// Light positioning
float lightX = 0.0f;
float lightZ = 0.0f;

// Light variables.
AmbientLight aLight(glm::vec3(1.0f, 1.0f, 1.0f),	// Ambient colour.
	0.2f);							// Ambient strength.

DirectionalLight dLight(glm::vec3(1.0f, 0.0f, 0.0f), // Direction.
	glm::vec3(1.0f, 1.0f, 0.5f),  // Diffuse colour.
	0.1f);						  // Diffuse strength.

PointLight pLight(glm::vec3(5.0f, 1.5f, -2.0f),	// Position.
	1.0f, 0.7f, 1.8f,				// Constant, Linear, Exponent.
	glm::vec3(1.0f, 0.0f, 1.0f),	// Diffuse colour.
	5.0f);						// Diffuse strength.

Material mat = { 0.1f, 32 }; // Alternate way to construct an object.

void timer(int);

void resetView()
{
	position = glm::vec3(5.0f, 3.0f, 10.0f);
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	pitch = 0.0f;
	yaw = -90.0f;
	// View will now get set only in transformObject
}

// ---------------------------- Shapes. Recommend putting in a map ---------------------------------------------
Grid g_grid(40);
Cube g_cube;
Cube longHedge;
Prism g_prism(24);
Plane g_plane;
ClonedCone g_clonedCone(12);
ClonedPrism g_clonedPrism(12);

void init(void)
{
	srand((unsigned)time(NULL));
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	glClearColor(0.0f, 0.2f, 1.0f, 1.0f);
	//Loading and compiling shaders
	program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up

	modelID = glGetUniformLocation(program, "model");
	projID = glGetUniformLocation(program, "projection");
	viewID = glGetUniformLocation(program, "view");

	// Projection matrix : 45∞ Field of View, aspect ratio, display range : 0.1 unit <-> 100 units
	Projection = glm::perspective(glm::radians(45.0f), 1.0f / 1.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	// Projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 100.0f); // In world coordinates

	// Camera matrix
	resetView();

	// Image loading.
	stbi_set_flip_vertically_on_load(true);

	unsigned char* image = stbi_load("d6.png", &width, &height, &bitDepth, 0);
	if (!image) cout << "Unable to load file!" << endl;

	glGenTextures(1, &alexTx);
	glBindTexture(GL_TEXTURE_2D, alexTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	// Note: image types with native transparency will need to be GL_RGBA instead of GL_RGB.
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image);

	// Second texture. Blank one.
	
	unsigned char* image2 = stbi_load("blank.jpg", &width, &height, &bitDepth, 0);
	if (!image2) cout << "Unable to load file!" << endl;
	
	glGenTextures(1, &blankTx);
	glBindTexture(GL_TEXTURE_2D, blankTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image2);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	//glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(image2);

	// Brick Texture
	unsigned char* image3 = stbi_load("brick.jpg", &width, &height, &bitDepth, 0);
	if (!image3) cout << "Unable to load file!" << endl;

	glGenTextures(1, &brickTx);
	glBindTexture(GL_TEXTURE_2D, brickTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image3);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Hedge Texture
	unsigned char* image4 = stbi_load("tx_grass_dull.png", &width, &height, &bitDepth, 0);
	if (!image4) cout << "Unable to load file!" << endl;

	glGenTextures(1, &hedgeTx);
	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image4);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

	// Setting ambient Light.
	glUniform3f(glGetUniformLocation(program, "aLight.ambientColour"), aLight.ambientColour.x, aLight.ambientColour.y, aLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.ambientStrength"), aLight.ambientStrength);

	// Setting directional light.
	glUniform3f(glGetUniformLocation(program, "dLight.base.diffuseColour"), dLight.diffuseColour.x, dLight.diffuseColour.y, dLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "dLight.base.diffuseStrength"), dLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "dLight.direction"), dLight.direction.x, dLight.direction.y, dLight.direction.z);

	// Setting point light.
	glUniform3f(glGetUniformLocation(program, "pLight.base.diffuseColour"), pLight.diffuseColour.x, pLight.diffuseColour.y, pLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLight.base.diffuseStrength"), pLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "pLight.position"), pLight.position.x, pLight.position.y, pLight.position.z);
	glUniform1f(glGetUniformLocation(program, "pLight.constant"), pLight.constant);
	glUniform1f(glGetUniformLocation(program, "pLight.linear"), pLight.linear);
	glUniform1f(glGetUniformLocation(program, "pLight.exponent"), pLight.exponent);

	vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

		ibo = 0;
		glGenBuffers(1, &ibo);
	
		points_vbo = 0;
		glGenBuffers(1, &points_vbo);

		colors_vbo = 0;
		glGenBuffers(1, &colors_vbo);

		uv_vbo = 0;
		glGenBuffers(1, &uv_vbo);

		normals_vbo = 0;
		glGenBuffers(1, &normals_vbo);

	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	// Change shape data.
	g_prism.SetMat(0.1, 16);

	// Enable depth test and blend.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	// Enable face culling.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);

	timer(0); 
}

//---------------------------------------------------------------------
//
// calculateView
//
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	View = glm::lookAt(
		position, // Camera position
		position + frontVec, // Look target
		upVec); // Up vector
	glUniform3f(glGetUniformLocation(program, "eyePosition"), position.x, position.y, position.z);
}

//---------------------------------------------------------------------
//
// transformModel
//
void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);
	
	calculateView();
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &View[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &Projection[0][0]);
}

void drawObjects()
{

}
//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	// Draw all shapes.

	// Grid Guideline
	glBindTexture(GL_TEXTURE_2D, blankTx);
	g_grid.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_LINE_STRIP, g_grid.NumIndices(), GL_UNSIGNED_SHORT, 0);


	glBindTexture(GL_TEXTURE_2D, brickTx);
	g_plane.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	g_plane.ColorShape(1.0f, 0.65f, 0.1f);
	transformObject(glm::vec3(10.0f, 10.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, g_plane.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glUniform3f(glGetUniformLocation(program, "pLight.position"), pLight.position.x, pLight.position.y, pLight.position.z);

	// Walls

	//glBindTexture(GL_TEXTURE_2D, blankTx);
	////g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	//transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -2.0f));
	//glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	//	

	// Hedge Maze Borders
	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferLongHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 23.0f), X_AXIS, 0.0f, glm::vec3(0.0f, 0.0f, -23.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferLongHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 23.0f), X_AXIS, 0.0f, glm::vec3(18.0f, 0.0f, -23.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferWideHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(7.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -1.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferWideHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(7.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(1.0f, 0.0f, -23.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferWideHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(7.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -1.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, hedgeTx);
	g_cube.BufferWideHedge(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(7.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(11.0f, 0.0f, -23.0f));
	glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);

	//------------------------- END Hedge borders ----------------------------------

	// Right Side
	for (int i = 0; i < 7; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(9.0f, 0.0f, -1.0f-i));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 5; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(10.0f + i, 0.0f, -7.0f));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(14.0f, 0.0f, -8.0f-i));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 4; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(11.0f + i, 0.0f, -10.0f));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	// Left Side

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(7.0f, 0.0f, -2.0f - i));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 5; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f + i, 0.0f, -3.0f));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f, 0.0f, -4.0 - i));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 4; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(5.0f + i, 0.0f, -5.0));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	for (int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 0.0f, -6.0 - i));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}


	for (int i = 0; i < 3; i++)
	{
		glBindTexture(GL_TEXTURE_2D, hedgeTx);
		g_cube.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
		transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(2.0f + i, 0.0f, -7.0f));
		glDrawElements(GL_TRIANGLES, g_cube.NumIndices(), GL_UNSIGNED_SHORT, 0);
	}

	// End Hedge Maze
	glBindTexture(GL_TEXTURE_2D, brickTx);
	g_clonedCone.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(5.0f, 1.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, g_clonedCone.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindTexture(GL_TEXTURE_2D, brickTx);
	g_clonedPrism.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, 0.0f, glm::vec3(3.0f, 1.0f, -2.0f));
	glDrawElements(GL_TRIANGLES, g_clonedPrism.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0); // Done writing.
	glutSwapBuffers(); // Now for a potentially smoother render.
}

void parseKeys()
{
	if (keys & KEY_FORWARD)
		position += frontVec * MOVESPEED;
	else if (keys & KEY_BACKWARD)
		position -= frontVec * MOVESPEED;
	if (keys & KEY_LEFT)
		position -= rightVec * MOVESPEED;
	else if (keys & KEY_RIGHT)
		position += rightVec * MOVESPEED;
	if (keys & KEY_UP)
		position.y += MOVESPEED;
	else if (keys & KEY_DOWN)
		position.y -= MOVESPEED;
}

void timer(int) { // essentially our update()
	parseKeys();
	glutPostRedisplay();
	glutTimerFunc(1000/FPS, timer, 0); // 60 FPS or 16.67ms.
}

//---------------------------------------------------------------------
//
// keyDown
//
void keyDown(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD; break;
	case 's':
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD; break;
	case 'a':
		if (!(keys & KEY_LEFT))
			keys |= KEY_LEFT; break;
	case 'd':
		if (!(keys & KEY_RIGHT))
			keys |= KEY_RIGHT; break;
	case 'r':
		if (!(keys & KEY_UP))
			keys |= KEY_UP; break;
	case 'f':
		if (!(keys & KEY_DOWN))
			keys |= KEY_DOWN; break;
	// Fill in point light movement.
	case 'i':
		pLight.position.z -= 0.1f;
		break;
	case 'j':
		pLight.position.x -= 0.1f;
		break;
	case 'k':
		pLight.position.z += 0.1f;
		break;
	case 'l':
		pLight.position.x += 0.1f;
		break;
	}
}

void keyDownSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		if (!(keys & KEY_FORWARD))
			keys |= KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		if (!(keys & KEY_BACKWARD))
			keys |= KEY_BACKWARD;
	}
}

void keyUp(unsigned char key, int x, int y) // x and y is mouse location upon key press.
{
	switch (key)
	{
	case 'w':
		keys &= ~KEY_FORWARD; break;
	case 's':
		keys &= ~KEY_BACKWARD; break;
	case 'a':
		keys &= ~KEY_LEFT; break;
	case 'd':
		keys &= ~KEY_RIGHT; break;
	case 'r':
		keys &= ~KEY_UP; break;
	case 'f':
		keys &= ~KEY_DOWN; break;
		// Fill in point light movement.
	case ' ':
		resetView();
	}
}

void keyUpSpec(int key, int x, int y) // x and y is mouse location upon key press.
{
	if (key == GLUT_KEY_UP)
	{
		keys &= ~KEY_FORWARD;
	}
	else if (key == GLUT_KEY_DOWN)
	{
		keys &= ~KEY_BACKWARD;
	}
}

void mouseMove(int x, int y)
{
	if (keys & KEY_MOUSECLICKED)
	{
		pitch += (GLfloat)((y - lastY) * TURNSPEED);
		yaw -= (GLfloat)((x - lastX) * TURNSPEED);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		keys |= KEY_MOUSECLICKED; // Flip flag to true
		glutSetCursor(GLUT_CURSOR_NONE);
		//cout << "Mouse clicked." << endl;
	}
	else
	{
		keys &= ~KEY_MOUSECLICKED; // Reset flag to false
		glutSetCursor(GLUT_CURSOR_INHERIT);
		//cout << "Mouse released." << endl;
	}
}

void clean()
{
	cout << "Cleaning up!" << endl;
	glDeleteTextures(1, &alexTx);
	glDeleteTextures(1, &blankTx);
}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);
	glutSetOption(GLUT_MULTISAMPLE, 8);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("GAME2012_A4_HoVincent");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.
	init();

	glutDisplayFunc(display);
	glutKeyboardFunc(keyDown);
	glutSpecialFunc(keyDownSpec);
	glutKeyboardUpFunc(keyUp); // New function for third example.
	glutSpecialUpFunc(keyUpSpec);

	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove); // Requires click to register.
	
	atexit(clean); // This GLUT function calls specified function before terminating program. Useful!

	glutMainLoop();

	return 0;
}
