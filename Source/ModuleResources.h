#pragma once

#include "Module.h"

class D3D12Module;

class ModuleResources : public Module
{
	ComPtr<ID3D12CommandAllocator> commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> commandList;

public:

	ModuleResources();
	~ModuleResources();

	bool init() override;

	ComPtr<ID3D12Resource> createUploadBuffer(size_t size, void* cpuData);
	ComPtr<ID3D12Resource> createDefaultBuffer(size_t size, void* data);
};