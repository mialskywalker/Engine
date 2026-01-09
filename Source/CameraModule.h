#pragma once

#include "Module.h"

class CameraModule : public Module
{
	const float MOVE_SPEED = 0.01f;
	bool cameraEnabled = true;

	Matrix model;
	Matrix view;
	float aspectRatio = 0.0f;
	float fov = 1.0f;

	Matrix projection;
	Matrix mvp;

	Vector3 startingPosition;
	Vector3 currentPosition;

	Quaternion startingRotation;
	Quaternion currentRotation;

	float yaw = 0.0f;
	float pitch = 0.0f;

	float mousePosX = 0.0f;
	float mousePosY = 0.0f;

	int speed = 1;
	int prevWheel = 0;

public:
	CameraModule();
	~CameraModule();

	bool init() override;
	void update() override;

	void setCameraEnabled(bool enabled);
	void setCameraFOV(float& FOV);
	void setModelMatrix(const Matrix& newModel);

	const float getFOV() { return this->fov; }
	const float getAspectRatio() { return this->aspectRatio; }
	const Matrix getView() { return this->view; }
	const Matrix getProjection() { return this->projection; }
	const Matrix getMVP() { return this->mvp; }
	const Matrix getModel() { return this->model; }
	const float getMoveSpeed() { return this->MOVE_SPEED; }
	const bool getCameraEnabled() { return this->cameraEnabled; }
};