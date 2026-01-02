#include "Globals.h"
#include "Exercise1.h"

#include "Application.h"
#include "D3D12Module.h"

void Exercise1::render()
{
	D3D12Module* d3d12 = app->getD3D12();
	ID3D12GraphicsCommandList4* commandList = d3d12->getCommandList();
	commandList->Reset(d3d12->getCurrentCommandAllocator(), nullptr);

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	float color[] = { 1.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(d3d12->getRTVCPUDescriptorHandle(), color, 0, nullptr);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);
	commandList->Close();

	ID3D12CommandList* lists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);
}