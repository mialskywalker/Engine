#pragma once

#include "Module.h"
#include <dxgi1_6.h>

class D3D12Module : public Module
{
	HWND hWnd = NULL;
	
	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;

	ComPtr<ID3D12InfoQueue> infoQueue;

	ComPtr<ID3D12CommandQueue> commandQueue;
	ComPtr<ID3D12CommandAllocator> commandAllocators[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12GraphicsCommandList4> commandList;

	ComPtr<IDXGISwapChain4> swapChain;
	ComPtr<ID3D12Resource> backBuffers[FRAMES_IN_FLIGHT];
	ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;

	ComPtr<ID3D12Fence> fence;
	HANDLE event = NULL;
	uint64_t fenceCounter = 0;
	uint64_t fenceValues[FRAMES_IN_FLIGHT] = { 0, 0, 0 };
	unsigned currentIndex = 0;

	unsigned windowWidth = 0;
	unsigned windowHeight = 0;

public:

	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;
	void preRender() override;
	void postRender() override;

	void resize();
	void flush();

	HWND getHwnd() { return hWnd; }
	ID3D12Device5* getDevice() { return device.Get(); }
	ID3D12CommandQueue* getCommandQueue() { return commandQueue.Get(); }
	ID3D12GraphicsCommandList* getCommandList() { return commandList.Get(); }
	ID3D12CommandAllocator* getCurrentCommandAllocator() { return commandAllocators[currentIndex].Get(); }
	ID3D12Resource* getCurrentBackBuffer() { return backBuffers[currentIndex].Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE getRenderTargetDescriptor();

	unsigned getWindowWidth() const { return windowWidth; }
	unsigned getWindowHeight() const { return windowHeight; }

private:

	void enableDebugLayer();
	void createFactory();
	void createDevice();
	void createInfoQueue();
	void createCommandQueue();
	void createCommandList();
	void createSwapChain();
	void createRTV();
	void createFence();

	unsigned getWindowSize(unsigned& width, unsigned& height);
};