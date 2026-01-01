#include "Globals.h"
#include "D3D12Module.h"

D3D12Module::D3D12Module(HWND hwnd) : hWnd(hwnd) {}

D3D12Module::~D3D12Module() {}

bool D3D12Module::init()
{
	bool succeed = false;

#if defined(_DEBUG)
	enableDebugLayer();
#endif
	succeed = SUCCEEDED(createFactory());
	succeed = succeed && SUCCEEDED(createDevice());
	setupInfoQueue();

	return succeed;
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