#pragma once

#include "Module.h"

class ImGuiPass;
class D3D12Module;
class Model;

class ModuleEditor : public Module
{
	ImGuiPass* imGuiPass = nullptr;

	static const int HISTORY_SIZE = 100;
	float fpsHistory[HISTORY_SIZE] = {};
	float msHistory[HISTORY_SIZE] = {};
	int historyIndex = 0;
	bool vsync = false;
	bool camera = true;
	
	float fov = 1.0f;
	int samplerIndex = 0;

public:
	ModuleEditor();
	~ModuleEditor();

	bool init() override;
	void update() override;
	void mainSettings();
	void modelOptions(Model& model);

	ImGuiPass* getImGui() { return this->imGuiPass; }

	const int& getSamplerIndex() const { return samplerIndex; }
	void setSamplerIndex(int& index) { samplerIndex = index; }

};