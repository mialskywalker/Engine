#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ModuleResources.h"

ModuleResources::ModuleResources() {};

ModuleResources::~ModuleResources() {};

bool ModuleResources::init()
{
	D3D12Module* d3d12 = app->getD3D12();
	bool succeed = SUCCEEDED(d3d12->getDevice()->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
	succeed = succeed && SUCCEEDED(d3d12->getDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));

	return succeed;
}

ComPtr<ID3D12Resource> ModuleResources::createUploadBuffer(size_t bufferSize, void* data)
{
	D3D12Module* d3d12 = app->getD3D12();

	ComPtr<ID3D12Resource> buffer;

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES props(D3D12_HEAP_TYPE_UPLOAD);
	d3d12->getDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
	
	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));

	memcpy(pData, data, bufferSize);

	buffer->Unmap(0, nullptr);

	return buffer;
}

ComPtr<ID3D12Resource> ModuleResources::createDefaultBuffer(size_t bufferSize, void* data)
{
	D3D12Module* d3d12 = app->getD3D12();

	ComPtr<ID3D12Resource> stagingBuffer;

	D3D12_RESOURCE_DESC stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES stagingProps(D3D12_HEAP_TYPE_UPLOAD);
	d3d12->getDevice()->CreateCommittedResource(&stagingProps, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));

	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0);
	stagingBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));

	memcpy(pData, data, bufferSize);

	stagingBuffer->Unmap(0, nullptr);


	ComPtr<ID3D12Resource> defaultBuffer;

	D3D12_RESOURCE_DESC defaultDesc = CD3DX12_RESOURCE_DESC::Buffer(bufferSize);
	CD3DX12_HEAP_PROPERTIES defaultProps(D3D12_HEAP_TYPE_DEFAULT);
	d3d12->getDevice()->CreateCommittedResource(&defaultProps, D3D12_HEAP_FLAG_NONE, &defaultDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&defaultBuffer));

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(1, &barrier);

	commandList->CopyResource(defaultBuffer.Get(), stagingBuffer.Get());

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* lists[] = { commandList.Get() };
	d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);

	d3d12->flush();
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	return defaultBuffer;
}
