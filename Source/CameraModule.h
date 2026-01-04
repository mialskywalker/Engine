#pragma once

#include "Module.h"

class CameraModule : public Module
{
	Matrix model;
	Vector3 eye;
	Vector3 target;
	Vector3 up;

	Matrix view;
	float aspectRatio;
	float fov;

	Matrix projection;
	Matrix mvp;

public:
	CameraModule();
	~CameraModule();

	bool init() override;
	void update() override;

	float getFOV() { return this->fov; }
	float getAspectRatio() { return this->aspectRatio; }
	Matrix getView() { return this->view; }
	Matrix getProjection() { return this->projection; }
	Matrix getMVP() { return this->mvp; }
};