#include <windows.h>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <conio.h>
using namespace std;

// include GLEW
#include "glew/glew.h"

// include GLFW
#include "glfw/glfw3.h"

// include tiny_obj_loader
#include "tiny_obj_loader\tiny_obj_loader.h"
using namespace tinyobj;

//include glm
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
using namespace glm;

//include SOIL
#include "SOIL.h"


static string objBasePath = "./obj/";
static string textureBasePath = "./textures/";

struct RenderData{
	std::vector<tinyobj::shape_t> objectData;
	std::vector<tinyobj::material_t> materialData;
	size_t windowData[2];
	GLuint programID;
	GLFWwindow * window;
	size_t objDataCount;
	GLuint vertexArray;
	GLuint *vertexBuffer;
	GLuint *normalBuffer;
	GLuint *indexBuffer;
	GLuint *textureBuffer;
	GLuint *texture;
	mat4 rotationMat, translationMat, scalingMat;
}info;

//Function Signatures
void setView();
void setBuffers();
void startRender();
void render3D();
void rotate(int);
void translate(int, int, int);
void updateView();


// Load and runtime compile GLSL shaders.
GLuint LoadShaders( const char* vertex_file_path, const char* fragment_file_path ){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader( GL_VERTEX_SHADER );
	GLuint FragmentShaderID = glCreateShader( GL_FRAGMENT_SHADER );

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}



	GLint Result = GL_FALSE;
	int InfoLogLength;



	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}



	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

GLuint loadTexture(string textureName){
	glUseProgram(info.programID);
	int width, height, channels = 0;;
	GLuint texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	unsigned char *textureData = SOIL_load_image((textureBasePath + textureName).c_str(), &width, &height, &channels, SOIL_LOAD_RGB);

	if(textureData == NULL)
		return 0;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
	return texture;
}

// Program initialization.
void initialize(string objName) {
	// create and compile our GLSL program from the shaders
	info.programID = LoadShaders( "shaders/vs.glsl", "shaders/fs.glsl" );

	string err;

	// Parse the Obj
	if(!LoadObj(info.objectData,info.materialData,err,(objBasePath + objName).c_str())){
		cout<<"\nError:"<<err;
		exit(1);
	}

	info.objDataCount = info.objectData.size();

	info.texture = new GLuint(info.objDataCount);

	for(size_t i = 0; i < info.objDataCount; i++){
		if(info.materialData[info.objectData[i].mesh.material_ids[0]].diffuse_texname != "")
			info.texture[i] = loadTexture(info.materialData[info.objectData[i].mesh.material_ids[0]].diffuse_texname);
		else
			info.texture[i] = 0;
	}
}

//Handle KeyPress
void handleKeyPress(GLFWwindow * window, int keyCode, int scanCode, int action, int mod){
	if(keyCode == GLFW_KEY_Q)
		rotate(5);
	else if(keyCode == GLFW_KEY_W)
		rotate(-5);
	else if(keyCode == GLFW_KEY_UP)
		translate(0,1,0);
	else if(keyCode == GLFW_KEY_DOWN)
		translate(0,-1,0);
	else if(keyCode == GLFW_KEY_LEFT)
		translate(-1,0,0);
	else if(keyCode == GLFW_KEY_RIGHT)
		translate(1,0,0);
	updateView();
}


//Main
int main( int argc, char** argv ) {

	// initialize GLFW
	if( !glfwInit() ) {
		fprintf( stderr, "Failed to initialize GLFW!\n" );
		return -1;
	}

	glfwWindowHint( GLFW_SAMPLES, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	info.windowData[0] = 1024;
	info.windowData[1] = 768;

	// open a window and create its OpenGL context
	info.window = glfwCreateWindow( info.windowData[0], info.windowData[1], "Assignment 3 - CG", NULL, NULL );
	if( info.window == NULL ) {
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible.\n" );
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent( info.window );

	// initialize GLEW
	glewExperimental = GL_TRUE; // Needed for core profile
	if( glewInit() != GLEW_OK ) {
		fprintf( stderr, "Failed to initialize GLEW\n" );
		return -1;
	}

	// ensure we can capture the escape key being pressed below
	glfwSetInputMode( info.window, GLFW_STICKY_KEYS, GL_TRUE );

	// set the GL clear color
	glClearColor( 0.f, 0.f, 0.f, 0.f );

	// load shaders and bind arrays
	initialize("vase.obj");

	setView();
	setBuffers();
	startRender();

	return 0;
}

void setView(){
	glUseProgram(info.programID);
	mat4 perspectiveMat = perspective(45.f, float(info.windowData[0]/info.windowData[1]), 1.f, 100.f);
	mat4 lookAtMat = lookAt(vec3(0,0,2), vec3(0,0,1), vec3(0,1,0));
	info.translationMat = translate(info.translationMat, vec3(0,-0.5,0));
	info.scalingMat = scale(info.scalingMat, vec3(0.05,0.05,0.05));
	mat4 transformMat = info.scalingMat * info.translationMat * info.rotationMat;
	mat4 normalMat = transpose(inverse(transformMat));
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "perspectiveMat"), 1, GL_FALSE, &perspectiveMat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "lookAtMat"), 1, GL_FALSE, &lookAtMat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "transformMat"), 1, GL_FALSE, &transformMat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "normalMat"), 1, GL_FALSE, &normalMat[0][0]);
}

void setBuffers(){
	glUseProgram(info.programID);
	glGenVertexArrays(1, &info.vertexArray);
	glBindVertexArray(info.vertexArray);
	info.vertexBuffer = new GLuint(info.objDataCount);
	info.indexBuffer = new GLuint(info.objDataCount);
	info.normalBuffer = new GLuint(info.objDataCount);
	info.textureBuffer = new GLuint(info.objDataCount);
	
	for(size_t i = 0; i < info.objDataCount; i++){
		glGenBuffers(1, &info.vertexBuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, info.vertexBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, info.objectData[i].mesh.positions.size()*sizeof(float), &info.objectData[i].mesh.positions[0], GL_STATIC_DRAW);

		glGenBuffers(1, &info.normalBuffer[i]);
		glBindBuffer(GL_ARRAY_BUFFER, info.normalBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, info.objectData[i].mesh.normals.size()*sizeof(float), &info.objectData[i].mesh.normals[0], GL_STATIC_DRAW);

		if(info.texture[i] != 0){
			glGenBuffers(1, &info.textureBuffer[i]);
			glBindBuffer(GL_ARRAY_BUFFER, info.textureBuffer[i]);
			glBufferData(GL_ARRAY_BUFFER, info.objectData[i].mesh.texcoords.size()*sizeof(float), &info.objectData[i].mesh.texcoords[0], GL_STATIC_DRAW);
		}

		glGenBuffers(1, &info.indexBuffer[i]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.indexBuffer[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, info.objectData[i].mesh.indices.size()*sizeof(unsigned int), &info.objectData[i].mesh.indices[0], GL_STATIC_DRAW);
	}
}

void startRender(){
	glClearColor(0.f,0.f,0.f,0.f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	glfwSetKeyCallback(info.window, handleKeyPress);
	do{
		render3D();
		glfwSwapBuffers(info.window);
		glfwPollEvents();
	}while(glfwGetKey( info.window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose( info.window ) == 0 );
	glfwTerminate();
}

void render3D(){
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(info.programID);

	int tFlag;

	for(size_t i = 0; i < info.objDataCount; i++){

		if(info.texture[i] == 0)
			tFlag = 0;
		else{
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, info.texture[i]);
			glProgramUniform1i(info.programID, glGetUniformLocation(info.programID, "texture"), 0);
			tFlag = 1;
		}


		glProgramUniform1i(info.programID, glGetUniformLocation(info.programID, "tFlag"), tFlag);

		glProgramUniform3fv(info.programID, glGetUniformLocation(info.programID, "ka"), 1,info.materialData[info.objectData[i].mesh.material_ids[0]].ambient);
		glProgramUniform3fv(info.programID, glGetUniformLocation(info.programID, "kd"), 1, info.materialData[info.objectData[i].mesh.material_ids[0]].diffuse);
		glProgramUniform3fv(info.programID, glGetUniformLocation(info.programID, "ks"), 1, info.materialData[info.objectData[i].mesh.material_ids[0]].specular);
		glProgramUniform1f(info.programID, glGetUniformLocation(info.programID, "n"), info.materialData[info.objectData[i].mesh.material_ids[0]].shininess);

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		if(tFlag == 1)
			glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, info.vertexBuffer[i]);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		
		glBindBuffer(GL_ARRAY_BUFFER, info.normalBuffer[i]);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		if(tFlag == 1){
			glBindBuffer(GL_ARRAY_BUFFER, info.textureBuffer[i]);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.indexBuffer[i]);
		
		glDrawElements(GL_TRIANGLES, info.objectData[i].mesh.indices.size(), GL_UNSIGNED_INT, (void*)0);
		
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		if(tFlag == 1)
			glDisableVertexAttribArray(2);
	}
}

//Other Functions

void rotate(int deg){
	info.rotationMat = rotate(info.rotationMat, (float)deg, vec3(0,1,0));
}

void translate(int distx, int disty, int distz){
	info.translationMat = translate(info.translationMat, vec3(distx,disty,distz));
}

void updateView(){
	mat4 transformMat =  info.scalingMat * info.translationMat * info.rotationMat;
	mat4 normalMat = transpose(inverse(transformMat));
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "transformMat"), 1, GL_FALSE, &transformMat[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(info.programID, "normalMat"), 1, GL_FALSE, &normalMat[0][0]);
}