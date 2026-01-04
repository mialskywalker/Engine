#pragma once

#include "Module.h"

class CameraModule : public Module
{
	const float MOVE_SPEED = 0.01f;

	Matrix model;
	Matrix view;
	float aspectRatio = 0.0f;
	float fov = 0.0f;

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

public:
	CameraModule();
	~CameraModule();

	bool init() override;
	void update() override;

	const float getFOV() { return this->fov; }
	const float getAspectRatio() { return this->aspectRatio; }
	const Matrix getView() { return this->view; }
	const Matrix getProjection() { return this->projection; }
	const Matrix getMVP() { return this->mvp; }
	const float getMoveSpeed() { return this->MOVE_SPEED; }
};