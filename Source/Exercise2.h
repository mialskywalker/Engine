#pragma once

#include "Module.h"

class Exercise2 : public Module
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12RootSignature> rootSignature;
	ComPtr<ID3D12PipelineState> pso;

public:
	bool init() override;
	void render() override;

private:
	bool createRootSignature();
	bool createPSO();
};