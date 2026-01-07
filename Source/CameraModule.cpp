#include "Globals.h"
#include "CameraModule.h"
#include "Application.h"
#include "D3D12Module.h"

#include "Keyboard.h"
#include "Mouse.h"

CameraModule::CameraModule() {}

CameraModule::~CameraModule() {}

bool CameraModule::init()
{
	D3D12Module* d3d12 = app->getD3D12();

	model = Matrix::Identity;

	startingPosition = Vector3(0.0f, 1.0f, 10.0f);
	startingRotation = Quaternion::Identity;
	currentRotation = Quaternion::CreateFromYawPitchRoll(Vector3(0.0f, 0.0f, 0.0f));
	currentPosition = startingPosition;

	view = Matrix::CreateFromQuaternion(currentRotation);
	view.Translation(-startingPosition);

	aspectRatio = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());

	projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, 0.1f, 1000.0f);
	mvp = (model * view * projection).Transpose();

	return true;
}

void CameraModule::update()
{
	D3D12Module* d3d12 = app->getD3D12();

	aspectRatio = float(d3d12->getWindowWidth()) / float(d3d12->getWindowHeight());

	Vector2 rotate = Vector2::Zero;

	Mouse& mouse = Mouse::Get();
	const Mouse::State& mouseState = mouse.GetState();

	Keyboard& keyboard = Keyboard::Get();
	const Keyboard::State& keyState = keyboard.GetState();

	float time = app->getElapsedMilis();

	if (cameraEnabled)
	{
		int mouseScrollWheel = mouseState.scrollWheelValue;

		if (mouseState.rightButton)
		{
			rotate.x = float(mousePosX - mouseState.x) * 0.15f;
			rotate.y = float(mousePosY - mouseState.y) * 0.15f;

			if (keyState.LeftShift)
				speed = 2;
			else
				speed = 1;

			if (keyState.W) currentPosition -= Vector3::Transform(Vector3(0, 0, 1), currentRotation) * MOVE_SPEED * time * speed;
			if (keyState.S) currentPosition += Vector3::Transform(Vector3(0, 0, 1), currentRotation) * MOVE_SPEED * time * speed;
			if (keyState.A) currentPosition -= Vector3::Transform(Vector3(1, 0, 0), currentRotation) * MOVE_SPEED * time * speed;
			if (keyState.D) currentPosition += Vector3::Transform(Vector3(1, 0, 0), currentRotation) * MOVE_SPEED * time * speed;
			if (keyState.Q) currentPosition -= Vector3::Transform(Vector3(0, 1, 0), currentRotation) * MOVE_SPEED * time * speed;
			if (keyState.E) currentPosition += Vector3::Transform(Vector3(0, 1, 0), currentRotation) * MOVE_SPEED * time * speed;

		}

		if (keyState.F)
		{
			currentPosition = startingPosition;
			currentRotation = startingRotation;
			yaw = 0.0f;
			pitch = 0.0f;
			return;
		}

		if (mouseScrollWheel > prevWheel) currentPosition -= Vector3::Transform(Vector3(0, 0, 1), currentRotation);

		if (mouseScrollWheel < prevWheel) currentPosition += Vector3::Transform(Vector3(0, 0, 1), currentRotation);
		prevWheel = mouseScrollWheel;
	}

	mousePosX = mouseState.x;
	mousePosY = mouseState.y;

	yaw += XMConvertToRadians(rotate.x);
	pitch += XMConvertToRadians(rotate.y);

	currentRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0.0f);	

	Quaternion inverse;
	currentRotation.Inverse(inverse);

	view = Matrix::CreateFromQuaternion(inverse);
	view.Translation(Vector3::Transform(-currentPosition, inverse));

	projection = Matrix::CreatePerspectiveFieldOfView(fov, aspectRatio, 0.1f, 1000.0f);
	mvp = (model * view * projection).Transpose();
}

void CameraModule::setCameraEnabled(bool enabled)
{
	this->cameraEnabled = enabled;
}

void CameraModule::setCameraFOV(float FOV)
{
	this->fov = FOV;
}

void CameraModule::setModelMatrix(float x, float y, float z)
{
	model = Matrix::CreateTranslation(Vector3(x, y, z));
}