#pragma once

#include "Module.h"
#include <dxgi1_6.h>

class D3D12Module : public Module
{
	HWND hWnd = NULL;

	ComPtr<ID3D12Debug> debugLayer;
	ComPtr<IDXGIFactory6> factory;
	ComPtr<IDXGIAdapter4> adapter;
	ComPtr<ID3D12Device5> device;
	ComPtr<ID3D12InfoQueue> infoQueue;

public:

	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;

private:

	void enableDebugLayer();
	bool createFactory();
	bool createDevice();
	void setupInfoQueue();
};

