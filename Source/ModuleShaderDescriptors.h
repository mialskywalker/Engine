#pragma once

#include "Module.h"

class ModuleShaderDescriptors : public Module
{
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	unsigned nextFreeSlotIndex = 0;
	unsigned descriptorSize = 0;
	unsigned capacity = 128;

public:
	ModuleShaderDescriptors();
	~ModuleShaderDescriptors();

	bool init() override;
	unsigned allocateDescriptor();
	unsigned createSRV(ID3D12Resource* resource);
	unsigned createNullSRV();
	void reset();

	ID3D12DescriptorHeap* getHeap() { return this->descriptorHeap.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE getCPUHandle(unsigned index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE getGPUHandle(unsigned index) const;
};