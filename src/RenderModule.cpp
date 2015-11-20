#include "RenderModule.h"


RenderModule::RenderModule(std::vector<tinyobj::shape_t> objectData, std::vector<tinyobj::material_t> materialData, size_t windowData)
{
	RenderModule::objectData = objectData;
	RenderModule::materialData = materialData;
	RenderModule::windowData = windowData;
}


RenderModule::~RenderModule(void)
{
	//Nothing for Now!
}
