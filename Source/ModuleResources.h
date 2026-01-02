#pragma once

#include "Module.h"

class ModuleResources : public Module
{
	ComPtr<ID3D12GraphicsCommandList4> commandList;
	ComPtr<ID3D12CommandAllocator> commandAllocator;

public:
	ModuleResources();
	~ModuleResources();

	bool init() override;

	ComPtr<ID3D12Resource> createUploadBuffer(size_t bufferSize, void* data);
	ComPtr<ID3D12Resource> createDefaultBuffer(size_t bufferSize, void* data);
};