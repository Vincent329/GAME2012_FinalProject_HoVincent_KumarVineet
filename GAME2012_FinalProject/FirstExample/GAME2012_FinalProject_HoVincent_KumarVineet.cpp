//***************************************************************************
// GAME2012_FinalProject_HoVincent_KumarVineet.cpp by Vincent Ho and Vineet Kumar (C) 2020 All Rights Reserved.
//
// Final Project Submission
//
// Description:
// 
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
#include <string>

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
GLuint alexTx, blankTx, brickTx, hedgeTx, groundTx, castleTx, doorTx, roofTx, stoneTx, bonusKeyTx, castleWallsTx;
GLint width, height, bitDepth;

// Light positioning
float lightX = 0.0f;
float lightZ = 0.0f;

// Light variables.
AmbientLight aLight(glm::vec3(1.0f, 1.0f, 1.0f),	// Ambient colour.
	0.25f);							// Ambient strength.

DirectionalLight dLight(glm::vec3(1.0f, 0.0f, 0.0f), // Direction.
	glm::vec3(1.0f, 1.0f, 0.5f),  // Diffuse colour.
	0.1f);						  // Diffuse strength.


// turn into an array of lights
//PointLight pLight(glm::vec3(5.0f, 1.5f, -2.0f),	// Position.
//	1.0f, 0.7f, 1.8f,				// Constant, Linear, Exponent.
//	glm::vec3(1.0f, 0.0f, 1.0f),	// Diffuse colour.
//	5.0f);						// Diffuse strength.

PointLight pLights[4] = { { glm::vec3(7.5f, 1.5f, -5.0f),	10.0f, glm::vec3(1.0f, 0.0f, 0.0f), 1.0f },
						  { glm::vec3(6.0f, 1.5f, -8.5f),	10.0f, glm::vec3(0.0f, 0.0f, 1.0f), 1.0f }, 
						  { glm::vec3(5.0f, 1.5f, -7.0f),	10.0f, glm::vec3(0.0f, 1.0f, 0.0f), 1.0f }, 
						  { glm::vec3(2.0f, 1.5f, -10.0f),	10.0f, glm::vec3(1.0f, 1.0f, 1.0f), 1.0f } };




Material mat = { 0.1f, 32 }; // Alternate way to construct an object.

// Bonus Key
glm::vec3 keyPosition_1, depositPosition_1 = { 9.0f, 0.5f, -12.0f };
glm::vec3 gatePosition_1 = { 9.0f, 0.25,-20 };
bool keyCollected_1 = false;
bool keyDeposited_1 = false;
bool printStatus_KeyCollected = false;
bool printStatus_KeyDeposited = false;
bool printStatus_NearGate = false;
bool hoverAnimation = false;

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
Prism g_prism(24);
Plane g_plane;
ClonedCone g_clonedCone(12,3);
ClonedPrism g_clonedPrism(12,3);
// where we set up objects
Cube test_cube(5.0f, 6.0f, 7.0f);

// creating the struct for shape info, create an std vector after
struct ShapeInfo {
	Shape shape;
	glm::vec3 scale;
	glm::vec3 color;
	glm::vec3 position;
	GLuint textureID;
	glm::vec3 rotAxis;
	float rotAngle;
};

std::vector<ShapeInfo> Shapes;

// creating any cube
void placeCube(glm::vec3 pos, float l, float w, float h, GLuint id, float scale, glm::vec3 c = glm::vec3(1, 1, 1), glm::vec3 rot = Y_AXIS, float angle = 0.0f)
{
	ShapeInfo s;
	s.shape = Cube(l, w, h);
	s.scale = glm::vec3(scale, scale, scale);
	s.color = c;
	s.position = pos;
	s.textureID = id;
	s.rotAxis = rot;
	s.rotAngle = angle;
	Shapes.push_back(s);	
}

// creating any cylinder
void placeCylinder(glm::vec3 pos, float sides, float h, GLuint id, glm::vec3 scale, 
	glm::vec3 c = glm::vec3(1, 1, 1), glm::vec3 rot = Y_AXIS, float angle = 0.0f)
{
	ShapeInfo s;
	s.shape = ClonedPrism(sides,h);
	s.scale = scale;
	s.color = c;
	s.position = pos;
	s.textureID = id;
	s.rotAxis = rot;
	s.rotAngle = angle;
	Shapes.push_back(s);
}

void placeCone(glm::vec3 pos, float sides, float h, GLuint id, glm::vec3 scale, 
	glm::vec3 c = glm::vec3(1, 1, 1), glm::vec3 rot = Y_AXIS, float angle = 0.0f)
{
	ShapeInfo s;
	s.shape = ClonedCone(sides,h);
	s.scale = scale;
	s.color = c;
	s.position = pos;
	s.textureID = id;
	s.rotAxis = rot;
	s.rotAngle = angle;
	Shapes.push_back(s);
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


void DrawShape(ShapeInfo& sInfo, GLenum mode = GL_TRIANGLES)
{
	glBindTexture(GL_TEXTURE_2D, sInfo.textureID);
	sInfo.shape.ColorShape(sInfo.color.x, sInfo.color.y, sInfo.color.z);
	sInfo.shape.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(sInfo.scale, sInfo.rotAxis, sInfo.rotAngle, sInfo.position);
	glDrawElements(mode, sInfo.shape.NumIndices(), GL_UNSIGNED_SHORT, 0);
}

void setupLights()
{

	// Setting ambient Light.
	glUniform3f(glGetUniformLocation(program, "aLight.ambientColour"), aLight.ambientColour.x, aLight.ambientColour.y, aLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "aLight.ambientStrength"), aLight.ambientStrength);

	// Setting directional light.
	glUniform3f(glGetUniformLocation(program, "dLight.base.diffuseColour"), dLight.diffuseColour.x, dLight.diffuseColour.y, dLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "dLight.base.diffuseStrength"), dLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "dLight.direction"), dLight.direction.x, dLight.direction.y, dLight.direction.z);

	// Setting point lights.

	glUniform3f(glGetUniformLocation(program, "pLights[0].base.diffuseColour"), pLights[0].diffuseColour.x, pLights[0].diffuseColour.y, pLights[0].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].base.diffuseStrength"), pLights[0].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[0].position"), pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[0].constant"), pLights[0].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[0].linear"), pLights[0].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[0].exponent"), pLights[0].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[1].base.diffuseColour"), pLights[1].diffuseColour.x, pLights[1].diffuseColour.y, pLights[1].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].base.diffuseStrength"), pLights[1].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[1].position"), pLights[1].position.x, pLights[1].position.y, pLights[1].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[1].constant"), pLights[1].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[1].linear"), pLights[1].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[1].exponent"), pLights[1].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[2].base.diffuseColour"), pLights[2].diffuseColour.x, pLights[2].diffuseColour.y, pLights[2].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[2].base.diffuseStrength"), pLights[2].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[2].position"), pLights[2].position.x, pLights[2].position.y, pLights[2].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[2].constant"), pLights[2].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[2].linear"), pLights[2].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[2].exponent"), pLights[2].exponent);

	glUniform3f(glGetUniformLocation(program, "pLights[3].base.diffuseColour"), pLights[3].diffuseColour.x, pLights[3].diffuseColour.y, pLights[3].diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "pLights[3].base.diffuseStrength"), pLights[3].diffuseStrength);
	glUniform3f(glGetUniformLocation(program, "pLights[3].position"), pLights[3].position.x, pLights[3].position.y, pLights[3].position.z);
	glUniform1f(glGetUniformLocation(program, "pLights[3].constant"), pLights[3].constant);
	glUniform1f(glGetUniformLocation(program, "pLights[3].linear"), pLights[3].linear);
	glUniform1f(glGetUniformLocation(program, "pLights[3].exponent"), pLights[3].exponent);
}

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
	
	unsigned char* image2 = stbi_load("floor.jpg", &width, &height, &bitDepth, 0);
	if (!image2) cout << "Unable to load file!" << endl;
	
	glGenTextures(1, &groundTx);
	glBindTexture(GL_TEXTURE_2D, groundTx);
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
	stbi_image_free(image3);

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
	stbi_image_free(image4);

	// Stone Texture
	unsigned char* image5 = stbi_load("tx_stone.png", &width, &height, &bitDepth, 0);
	if (!image5) cout << "Unable to load file!" << endl;

	glGenTextures(1, &stoneTx);
	glBindTexture(GL_TEXTURE_2D, stoneTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image5);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	stbi_image_free(image5);

	// Bonus Key
	unsigned char* image6 = stbi_load("tx_bonus_key.png", &width, &height, &bitDepth, 0);
	if (!image6) cout << "Unable to load file!" << endl;

	glGenTextures(1, &bonusKeyTx);
	glBindTexture(GL_TEXTURE_2D, bonusKeyTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image6);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Castle Walls Key
	unsigned char* image7 = stbi_load("tx_brick_bright.jpg", &width, &height, &bitDepth, 0);
	if (!image7) cout << "Unable to load file!" << endl;

	glGenTextures(1, &castleWallsTx);
	glBindTexture(GL_TEXTURE_2D, castleWallsTx);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image7);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(program, "texture0"), 0);

	// Set lights
	setupLights();
		

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

	// ------------------------- BEGIN HEDGE MAZE -----------------------------

	// Left Side
	placeCube(glm::vec3(2.25f, 0.0f, -2.75f), 0.5f, 5.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(7.25f, 0.0f, -2.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -5.0f), 2.25f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(5.25f, 0.0f, -4.75f), 0.5f, 4.0f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(5.25f, 0.0f, -6.75f), 2.0f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -6.75f), 0.5f, 3.0f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -13.75f), 5.5f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(1.0f, 0.0f, -13.75f), 0.5f, 1.25f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(4.25, 0.0f, -10.75f), 3.0f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(4.75, 0.0f, -10.75f), 0.5f, 1.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(4.75f, 0.0f, -12.75f), 0.5f, 1.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(4.25f, 0.0f, -15.75f), 3.5f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(6.25f, 0.0f, -8.75f), 0.5f, 2.75f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(7.25f, 0.0f, -8.25f), 2.25f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(6.25f, 0.0f, -15.75f), 7.0f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -15.75f), 0.5f, 2.0f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -17.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(2.25f, 0.0f, -17.75), 0.5f, 5.75f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(0.75f, 0.0f, -20.75f), 0.5f, 4.0f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(6.0f, 0.0f, -19.75), 0.5f, 3.75f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(9.25f, 0.0f, -22.0f), 2.25f, 0.5f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(6.75f, 0.0f, -14.75f), 0.5f, 2.25f, 1.0f, hedgeTx, 1.0f);

	// middle
	placeCube(glm::vec3(9.25f, 0.0f, -6.75f), 6.75f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(9.0f, 0.0f, -16.75f), 0.5f, 2.75f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(11.25f, 0.0f, -18.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(11.25f, 0.0f, -18.75f), 0.5f, 2.75f, 1.0f, hedgeTx, 1.0f );

	// right side
	placeCube(glm::vec3(11.25f, 0.0f, -4.75f), 4.0f, 0.5f, 1.0f,  hedgeTx, 1.0f );
	placeCube(glm::vec3(11.75f, 0.0f, -4.75f), 0.5f, 5.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(13.25f, 0.0f, -2.75f), 0.5f, 3.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(16.25f, 0.0f, -4.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(9.75f, 0.0f, -6.75f), 0.5f, 5.0f, 1.0f, hedgeTx, 1.0f);
	placeCube(glm::vec3(14.25f, 0.0f, -8.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(10.0f, 0.0f, -8.75f), 0.5f, 4.75f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(12.25f, 0.0f, -14.25f), 5.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(10.0f, 0.0f, -14.75f), 0.5f, 3.75f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(13.25f, 0.0f, -17.0f), 2.25f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(16.25f, 0.0f, -10.75f), 4.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(14.25f, 0.0f, -10.75f), 0.5f, 2.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(14.25f, 0.0f, -12.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(14.25f, 0.0f, -12.75f), 0.5f, 4.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(15.25f, 0.0f, -14.75f), 0.5f, 3.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(15.25f, 0.0f, -16.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(15.25f, 0.0f, -16.75f), 0.5f, 1.75f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(15.25f, 0.0f, -18.75f), 0.5f, 3.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(16.25f, 0.0f, -20.25f), 1.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(11.25f, 0.0f, -20.75f), 0.5f, 5.5f, 1.0f, hedgeTx, 1.0f );

	// borders
	placeCube(glm::vec3(0.25f, 0.0f, -22.75f), 22.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(18.25f, 0.0f, -22.75f), 22.5f, 0.5f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(0.75f, 0.0f, -0.75), 0.5f, 7.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(0.75f, 0.0f, -22.75f), 0.5f, 7.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(11.25f, 0.0f, -0.75f), 0.5f, 7.0f, 1.0f, hedgeTx, 1.0f );
	placeCube(glm::vec3(11.25f, 0.0f, -22.75f), 0.5f, 7.0f, 1.0f, hedgeTx, 1.0f );

	// stone platform
	placeCube(glm::vec3(8.0f, 0.0f, -13.5f), 4.0f, 3.0f, 0.5f, stoneTx, 1.0f);
	// ------------------------ End Hedge Maze --------------------------------


	// ------------------------ Castle Walls --------------------------------
	//Left 
	placeCube(glm::vec3(-2.0f, 0.0f, -22.75f), 23.0f, 1.5f, 4.0f, castleWallsTx, 1.0f);

	placeCube(glm::vec3(-1.0f, 4.0f, -22.75f), 23.0f, 0.5f, 0.25f, castleWallsTx, 1.0f);
	placeCube(glm::vec3(-2.0f, 4.0f, -22.75f), 23.0f, 0.5f, 0.25f, castleWallsTx, 1.0f);

	// battlements - merlons and crenels
	float merlon_1 = -0.75f;
	for(int i = 0; i < 16; i++, merlon_1 -= 1.5f)
	{
		placeCube(glm::vec3(-1.0f, 4.25f, merlon_1), 1.0f, 0.5f, 0.75f, castleWallsTx, 1.0f);
		placeCube(glm::vec3(-2.0f, 4.25f, merlon_1), 1.0f, 0.5f, 0.75f, castleWallsTx, 1.0f);
	}
	// ------------------------ Castle Corners --------------------------------
	placeCylinder(glm::vec3(-2.25f, 0.0f, -0.1f), 12, 5, castleWallsTx, { 2.0f,1.0f,2.0f, }, { 1.0f,1.0f,1.0f }, { 0,1,0 }, 0);
	placeCone(glm::vec3(-2.75f, 5.0f, -0.5f), 10, 3, castleWallsTx, { 3.0f,1.0f,3.0f }, { 1.0f,1.0f,1.0f }, { 0,1,0 }, 0);
	placeCylinder(glm::vec3(-2.25f, 0.0f, -24.25f), 12, 5, castleWallsTx, { 2.0f,1.0f,2.0f, }, { 1.0f,1.0f,1.0f }, { 0,1,0 }, 0);
	placeCone(glm::vec3(-2.75f, 5.0f, -25.0f), 10, 3, castleWallsTx, { 3.0f,1.0f,3.0f }, { 1.0f,1.0f,1.0f }, { 0,1,0 }, 0);
	

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
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(vao);
	// Draw all shapes.

	// Grid Guideline
	glBindTexture(GL_TEXTURE_2D, groundTx);
	g_grid.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	transformObject(glm::vec3(1.0f, 1.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_LINE_STRIP, g_grid.NumIndices(), GL_UNSIGNED_SHORT, 0);


	glBindTexture(GL_TEXTURE_2D, groundTx);
	g_plane.BufferShape(&ibo, &points_vbo, &colors_vbo, &uv_vbo, &normals_vbo, program);
	g_plane.ColorShape(1.0f, 0.65f, 0.1f);
	transformObject(glm::vec3(21.0f, 27.0f, 1.0f), X_AXIS, -90.0f, glm::vec3(-1.0f, 0.0f, 2.0f));
	glDrawElements(GL_TRIANGLES, g_plane.NumIndices(), GL_UNSIGNED_SHORT, 0);

	glUniform3f(glGetUniformLocation(program, "pLights[0].position"), pLights[0].position.x, pLights[0].position.y, pLights[0].position.z);
	glUniform3f(glGetUniformLocation(program, "pLights[1].position"), pLights[1].position.x, pLights[1].position.y, pLights[1].position.z);
	glUniform3f(glGetUniformLocation(program, "pLights[2].position"), pLights[2].position.x, pLights[2].position.y, pLights[2].position.z);
	glUniform3f(glGetUniformLocation(program, "pLights[3].position"), pLights[3].position.x, pLights[3].position.y, pLights[3].position.z);

	// ------------------------Bonus Part --------------------------------
	// Start here
	// keep on comparing position of camera and key
	if (!keyCollected_1)
	{
		//std::cout << "Shapes size = " << Shapes.size() << std::endl;
		keyPosition_1 = { 15.25f, 0.0f, -12.0f };
	
		//placeCube(keyPosition_1, 1.0f, 1.0f, 1.0f, bonusKeyTx, 1);

		if (abs(position.x - keyPosition_1.x) < 2.0f && abs(position.z - keyPosition_1.z) < 2.0f && abs(position.y - keyPosition_1.y) < 2.0f)
		{
			//std::cout << " In Collision! " << std::endl;
			printStatus_KeyCollected = true;
			keyCollected_1 = true;

		}
	}

	if (keyCollected_1 && !keyDeposited_1)
	{
		for (int i = 0; i < Shapes.size(); i++)
		{
			if (Shapes[i].textureID == bonusKeyTx)
			{
				Shapes.erase(Shapes.begin() + i);
				//break;
			}
		}
		if (abs(position.x - depositPosition_1.x) < 2.0f && abs(position.z - depositPosition_1.z) < 2.0f && abs(position.y - depositPosition_1.y) < 2.0f)
		{
			//std::cout << " You have the key, Deposit it! " << position.x << std::endl;
			keyDeposited_1 = true;
			printStatus_KeyDeposited = true;
		}
	}


	if (keyDeposited_1)
	{
		//placeCube(depositPosition_1, 1.0f, 1.0f, 1.0f, bonusKeyTx, 1.0f);
		//std::cout << "Ready to Go !" << std::endl;
	}

	// Draw all the shapes
	for (std::vector<ShapeInfo>::iterator it = Shapes.begin(); it != Shapes.end(); ++it) {
		DrawShape(*it);
		//std::cout << Shapes.size() << std::endl;
	}



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

	if (printStatus_KeyCollected)
	{
		std::cout << "Key Collected!! Now Deposit this key at it's location." << std::endl;
		printStatus_KeyCollected = false;
	}

	if(printStatus_KeyDeposited)
	{
		std::cout << "Key Deposited!! The Gates are now unlocked." << std::endl;
		printStatus_KeyDeposited = false;
	}

	if(printStatus_NearGate)
	{
		
	}
	
	if(abs(position.x - gatePosition_1.x) < 2.0f && abs(position.z - gatePosition_1.z) < 2.0f && abs(position.y - gatePosition_1.y) < 2.0f)
	{
		//std::system("cls");
		if (!keyCollected_1)
		{
			std::cout << "Gates are Locked! There is a key somewhere over here. Collect it, deposit it at the room to unlock these gates." << std::endl;
		}
		else if (keyCollected_1 && !keyDeposited_1)
		{
			std::cout << "Key Has been Collected! Now place it at it's location to unlock the gate." << std::endl;
		}
		else if (keyDeposited_1)
		{
			std::cout << "You may pass!" << std::endl;
		}
	}
	
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
		for (int i = 0; i < 4; i++)
		pLights[i].position.z -= 0.1f;
		break;
	case 'j':
		for (int i = 0; i < 4; i++)
		pLights[i].position.x -= 0.1f;
		break;
	case 'k':
		for (int i = 0; i < 4; i++)
		pLights[i].position.z += 0.1f;
		break;
	case 'l':
		for (int i = 0; i < 4; i++)
		pLights[i].position.x += 0.1f;
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
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("GAME2012_FinalProject_HoVincent_KumarVineet");

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
