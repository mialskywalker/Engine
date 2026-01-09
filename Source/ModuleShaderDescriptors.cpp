#include "Globals.h"
#include "ModuleShaderDescriptors.h"
#include "Application.h"
#include "D3D12Module.h"

ModuleShaderDescriptors::ModuleShaderDescriptors() {}

ModuleShaderDescriptors::~ModuleShaderDescriptors() {}

bool ModuleShaderDescriptors::init()
{

	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NumDescriptors = capacity;

	app->getD3D12()->getDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap));
	descriptorSize = app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

unsigned ModuleShaderDescriptors::allocateDescriptor()
{
	unsigned index = 0;
	if (nextFreeSlotIndex < capacity)
	{
		index = nextFreeSlotIndex++;
	}
	return index;
}

unsigned ModuleShaderDescriptors::createSRV(ID3D12Resource* resource)
{
	unsigned index = allocateDescriptor();
	
	D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
	app->getD3D12()->getDevice()->CreateShaderResourceView(resource, nullptr, handle);

	return index;
}

unsigned ModuleShaderDescriptors::createNullSRV()
{
	unsigned index = allocateDescriptor();

	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Texture2D.MostDetailedMip = 0;
	desc.Texture2D.MipLevels = 1;
	desc.Texture2D.ResourceMinLODClamp = 0.0f;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = getCPUHandle(index);
	app->getD3D12()->getDevice()->CreateShaderResourceView(nullptr, &desc, handle);

	return index;
}

void ModuleShaderDescriptors::reset()
{
	nextFreeSlotIndex = 0;
}

D3D12_CPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getCPUHandle(unsigned index) const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
}

D3D12_GPU_DESCRIPTOR_HANDLE ModuleShaderDescriptors::getGPUHandle(unsigned index) const
{
	return CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, app->getD3D12()->getDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
}