#include "Globals.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module()
{
	flush();
}

bool D3D12Module::init()
{
	bool success = false;

	getWindowSize(windowWidth, windowHeight);

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	success = createFactory();
	success = success && createDevice();

#if defined(_DEBUG)
	createInfoQueue();
#endif

	success = success && createCommandQueue();
	success = success && createCommandList();
	success = success && createSwapChain();
	success = success && createRTV();
	success = success && createFence();
	success = success && createDepthStencil();

	return success;
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

bool D3D12Module::createFactory()
{
	bool success = false;

#if defined(_DEBUG)
	success = SUCCEEDED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory)));
#else
	success = SUCCEEDED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
#endif

	return success;
}

bool D3D12Module::createDevice()
{
	bool success = false;

	factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
	success = SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
	
	return success;
}

void D3D12Module::createInfoQueue()
{
	device.As(&infoQueue);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
}

bool D3D12Module::createCommandQueue()
{
	bool success = false;

	D3D12_COMMAND_QUEUE_DESC desc = {};

	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	success = SUCCEEDED(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&commandQueue)));

	return success;
}

bool D3D12Module::createCommandList()
{
	bool success = false;

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocators[i]));
	}
	
	success = SUCCEEDED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&commandList)));
	commandList->Close();

	return success;
}

bool D3D12Module::createSwapChain()
{
	bool success = false;

	DXGI_SWAP_CHAIN_DESC1 desc = {};

	desc.Width = windowWidth;
	desc.Height = windowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	desc.Stereo = FALSE;
	desc.SampleDesc = { 1, 0 };
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = FRAMES_IN_FLIGHT;

	desc.Scaling = DXGI_SCALING_NONE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	desc.Flags = 0;

	ComPtr<IDXGISwapChain1> swapChain1;
	success = SUCCEEDED(factory->CreateSwapChainForHwnd(commandQueue.Get(), hWnd, &desc, nullptr, nullptr, &swapChain1));
	success = success && SUCCEEDED(swapChain1.As(&swapChain));

	return success;
}

bool D3D12Module::createRTV()
{
	bool success = false;

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.NumDescriptors = FRAMES_IN_FLIGHT;
	
	success = SUCCEEDED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvDescriptorHeap)));

	D3D12_CPU_DESCRIPTOR_HANDLE rtvCPUHandle(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	unsigned descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffers[i]));
		device->CreateRenderTargetView(backBuffers[i].Get(), nullptr, rtvCPUHandle);
		rtvCPUHandle.ptr += descriptorSize;
	}

	return success;
}

bool D3D12Module::createFence()
{
	bool success = false;

	device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	event = CreateEvent(NULL, FALSE, FALSE, NULL);
	success = event != NULL;

	return success;
}

bool D3D12Module::createDepthStencil()
{
	bool success = false;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight,
		1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	success = SUCCEEDED(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthStencilBuffer)));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = 1;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	success = success && SUCCEEDED(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&dsvDescriptorHeap)));

	device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	return success;
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::getRenderTargetDescriptor()
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), currentIndex, device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV));
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12Module::getDepthStencilDescriptor()
{
	return dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

unsigned D3D12Module::getWindowSize(unsigned& width, unsigned& height)
{
	RECT client = {};
	GetClientRect(hWnd, &client);

	width = unsigned(client.right - client.left);
	height = unsigned(client.bottom - client.top);
	return width, height;
}

void D3D12Module::resize()
{
	unsigned w, h;
	getWindowSize(w, h);

	if (w != windowWidth || h != windowHeight)
	{
		windowWidth = w;
		windowHeight = h;
		flush();

		for (unsigned i = 0; i < FRAMES_IN_FLIGHT; ++i)
		{
			backBuffers[i].Reset();
			fenceValues[i] = 0;
		}

		DXGI_SWAP_CHAIN_DESC desc = {};

		swapChain->GetDesc(&desc);
		swapChain->ResizeBuffers(FRAMES_IN_FLIGHT, windowWidth, windowHeight, desc.BufferDesc.Format, desc.Flags);

		if (windowWidth > 0 && windowHeight > 0)
		{
			createRTV();
			createDepthStencil();
		}
	}

}

void D3D12Module::flush()
{
	commandQueue->Signal(fence.Get(), ++fenceCounter);
	fence->SetEventOnCompletion(fenceCounter, event);
	WaitForSingleObject(event, INFINITE);
}