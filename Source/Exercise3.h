#pragma once

#include "Module.h"
#include "ImGuiPass.h"

class DebugDrawPass;

class Exercise3 : public Module
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;
	DebugDrawPass* debugDraw = nullptr;
	ImGuiPass* imgui = nullptr;

public:
	~Exercise3();

	bool init() override;
	void render() override;
	void preRender() override;

private:
	bool createRootSignature();
	bool createPSO();
};