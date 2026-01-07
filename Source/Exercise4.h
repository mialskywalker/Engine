#pragma once

#include "Module.h"
#include "ImGuiPass.h"

class DebugDrawPass;

class Exercise4 : public Module
{
	ComPtr<ID3D12Resource> texture;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
	DebugDrawPass* debugDraw = nullptr;
	ImGuiPass* imgui = nullptr;
	unsigned index = 0;
	int samplerIndex = 0;

	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;

public:
	Exercise4();
	~Exercise4();

	bool init() override;
	void render() override;
	void preRender() override;
	void update() override;

private:
	bool createRootSignature();
	bool createPSO();
};