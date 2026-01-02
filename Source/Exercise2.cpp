#include "Globals.h"
#include "Exercise2.h"
#include "Application.h"
#include "ModuleResources.h"

bool Exercise2::init()
{

	struct Vertex
	{
		float x, y, z;
	};

	Vertex vertices[3] =
	{
		{ -1.0f, -1.0f, 0.0f },
		{  0.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f }
	};

	vertexBufferView = app->getResources()->createDefaultBuffer(sizeof(vertices), vertices);

	return true;
}