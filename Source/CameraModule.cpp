#include "Globals.h"
#include "CameraModule.h"
#include "Application.h"
#include "D3D12Module.h"

#include "Keyboard.h"

CameraModule::CameraModule() {}

CameraModule::~CameraModule() {}

bool CameraModule::init()
{
	D3D12Module* d3d12 = app->getD3D12();

	model = Matrix::Identity;
	eye = Vector3(0.0f, 10.0f, 10.f);
	target = Vector3::Zero;
	up = Vector3::Up;

	view = Matrix::CreateLookAt(eye, target, up);
	aspectRatio = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());
	fov = XM_PIDIV4;

	projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, 0.1f, 1000.0f);
	mvp = (model * view * projection).Transpose();

	return true;
}

void CameraModule::update()
{
	D3D12Module* d3d12 = app->getD3D12();

	aspectRatio = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());

	Keyboard& keyboard = Keyboard::Get();

	const Keyboard::State& keyState = keyboard.GetState();

	if (keyState.F)
	{
		fov = XM_PIDIV2;
	}
	else if (keyState.G)
	{
		fov = XM_PIDIV4;
	}

	projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, 0.1f, 1000.0f);
	mvp = (model * view * projection).Transpose();
}