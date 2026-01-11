#pragma once

#include "Module.h"
#include <filesystem>

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
	ComPtr<ID3D12Resource> createTextureFromFile(const std::filesystem::path& filePath);
	ComPtr<ID3D12Resource> createRenderTarget(DXGI_FORMAT format, unsigned width, unsigned height, const float clearColor[4]);
	ComPtr<ID3D12Resource> createDepthStencil(DXGI_FORMAT format, unsigned width, unsigned height, float clearValue);
};