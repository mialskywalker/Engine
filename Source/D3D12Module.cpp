#include "Globals.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module() {}

bool D3D12Module::init()
{
	windowWidth = 1280;
	windowHeight = 720;

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	createFactory();
	createDevice();

#if defined(_DEBUG)
	createInfoQueue();
#endif

	createCommandQueue();
	createCommandList();
	createSwapChain();
	createRTV();
	createFence();

	return true;
}

void D3D12Module::preRender()
{
	currentIndex = swapChain->GetCurrentBackBufferIndex();
	if (fenceValues[currentIndex] != 0)
	{
		fence->SetEventOnCompletion(fenceValues[currentIndex], event);
		WaitForSingleObject(event, INFINITE);
		fenceValues[currentIndex];
	}
	commandAllocators[currentIndex]->Reset();
}

void D3D12Module::postRender()
{
	swapChain->Present(0, 0);
	fenceValues[currentIndex] = ++fenceCounter;
	commandQueue->Signal(fence.Get(), fenceValues[currentIndex]);
}

void D3D12Module::enableDebugLayer()
{
	ComPtr<ID3D12Debug> debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();
}

void D3D12Module::createFactory()
{

#if defined(_DEBUG)
	CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
	CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif

}

void D3D12Module::createDevice()
{
	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
}

void D3D12Module::createInfoQueue()
{
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

void D3D12Module::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue));
}

void D3D12Module::createCommandList()
{
	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
	}
	
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList));
	commandList->Close();
}

void D3D12Module::createSwapChain()
{
	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAMES_IN_FLIGHT;

	desc.Scaling = DXGI_SCALING_STRETCH;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1);
	swapChain1.As(&swapChain);
}

void D3D12Module::createRTV()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = FRAMES_IN_FLIGHT;
	
	device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	unsigned descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvCPUHandle);
		rtvCPUHandle.ptr += descriptorSize;
	}
}

void D3D12Module::createFence()
{
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
}