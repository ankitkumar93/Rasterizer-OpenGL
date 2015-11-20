#include "RenderModule.h"
using namespace std;
using namespace tinyobj;
using namespace glm;


RenderModule::RenderModule(vector<shape_t> objectData, vector<material_t> materialData, size_t *windowData, GLuint programID,GLFWwindow * window)
{
	RenderModule::objectData = objectData;
	RenderModule::materialData = materialData;
	RenderModule::windowData = windowData;
	RenderModule::programID = programID;
	RenderModule::window = window;
	RenderModule::objDataCount = objectData.size();
	RenderModule::rotationMat = mat4();
	RenderModule::translationMat = mat4();
	RenderModule::scalingMat = mat4();
}


RenderModule::~RenderModule(void)
{
	//Nothing for Now!
}


