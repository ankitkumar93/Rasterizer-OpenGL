#include <iostream>
#include <vector>
#include "tiny_obj_loader\tiny_obj_loader.h"
// include GLEW
#include "glew/glew.h"

// include GLFW
#include "glfw/glfw3.h"

// include GLM
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma once
class RenderModule
{
private:
	std::vector<tinyobj::shape_t> objectData;
	std::vector<tinyobj::material_t> materialData;
	size_t* windowData;
	GLuint programID;
	GLFWwindow * window;
	size_t objDataCount;
	GLuint vertexArray;
	GLuint *vertexBuffer;
	GLuint *normalBuffer;
	GLuint *indexBuffer;
	glm::mat4 rotationMat, translationMat, scalingMat;
	void render3D();
	void updateView();
public:
	RenderModule(std::vector<tinyobj::shape_t>, std::vector<tinyobj::material_t>,size_t*,GLuint,GLFWwindow *);
	~RenderModule(void);
	void setView();
	void setBuffers();
	void startRender();
	void rotate(int);
};

