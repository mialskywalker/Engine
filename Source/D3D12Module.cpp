#include "Globals.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module() {}

bool D3D12Module::init()
{
	bool succeed = false;

	getWindowSize(windowWidth, windowHeight);

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	succeed = createFactory();
	succeed = succeed && createDevice();
	setupInfoQueue();
	succeed = succeed && createCommandQueue();
	succeed = succeed && createSwapChain();
	succeed = succeed && createRTVDescriptorHeap();
	createRTV();
	succeed = succeed && createCommandAllocators();
	succeed = succeed && createCommandList();
	succeed = succeed && createFence();

	return succeed;
}

void D3D12Module::preRender()
{
	currentBufferIndex = swapChain->GetCurrentBackBufferIndex();
	if (fenceValues[currentBufferIndex] != 0)
	{
		fence->SetEventOnCompletion(fenceValues[currentBufferIndex], event);
		WaitForSingleObject(event, INFINITE);
	}
	commandAllocators[currentBufferIndex]->Reset();
	commandList->Reset(commandAllocators[currentBufferIndex].Get(), nullptr);
}

void D3D12Module::postRender()
{
	swapChain->Present(0, 0);
	fenceValues[currentBufferIndex] = ++fenceValue;
	commandQueue->Signal(fence.Get(), fenceValues[currentBufferIndex]);
}

void D3D12Module::enableDebugLayer()
{
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer));
	debugLayer->EnableDebugLayer();
}

bool D3D12Module::createFactory()
{
	bool succeed = false;
#if defined(_DEBUG)
	succeed = SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));
#else
	succeed = SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
#endif

	return succeed;
}

bool D3D12Module::createDevice()
{

	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	bool succeed = SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));

	return succeed;
}

void D3D12Module::setupInfoQueue()
{
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

bool D3D12Module::createCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	bool succeed = SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

	return succeed;
}

bool D3D12Module::createSwapChain()
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
	bool succeed = SUCCEEDED(factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1));

	succeed = succeed && SUCCEEDED(swapChain1.As(&swapChain));

	return succeed;
}

bool D3D12Module::createRTVDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FRAMES_IN_FLIGHT;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	bool succeed = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&RTVdescriptorHeap)));

	return succeed;
}

void D3D12Module::createRTV()
{
	D3D12_CPU_DESCRIPTOR_HANDLE handle(RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	unsigned descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, handle);
		handle.ptr += descriptorSize;
	}
}

bool D3D12Module::createCommandAllocators()
{
	bool succeed = false;

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		succeed = SUCCEEDED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i])));
	}

	return succeed;
}

bool D3D12Module::createCommandList()
{
	bool succeed = false;

	succeed = SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)));
	commandList->Close();

	return succeed;
}

bool D3D12Module::createFence()
{
	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	
	bool succeed = event != NULL;
	return succeed;
}

unsigned D3D12Module::getWindowSize(unsigned& width, unsigned& height)
{
	RECT rect = {};
	GetClientRect(hWnd, &rect);

	width = unsigned(rect.right - rect.left);
	height = unsigned(rect.bottom - rect.top);
	
	return width, height;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::getRTDescriptor()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(RTVdescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentBufferIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}