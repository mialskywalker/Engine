#include "Globals.h"
#include "Assignment1.h"

#include "Application.h"
#include "D3D12Module.h"
#include "ModuleCamera.h"
#include "ModuleResources.h"
#include "ReadData.h"

#include <d3d12.h>
#include "d3dx12.h"

bool Assignment1::init()
{
	D3D12Module* d3d12 = app->getD3D12();
	debugDrawPass = std::make_unique<DebugDrawPass>(d3d12->getDevice(), d3d12->getCommandQueue(), false);

	return true;
}

void Assignment1::render()
{
	D3D12Module* d3d12 = app->getD3D12();
	ID3D12GraphicsCommandList* commandList = d3d12->getCommandList();
	ModuleCamera* camera = app->getCamera();

	commandList->Reset(d3d12->getCurrentCommandAllocator(), nullptr);
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	commandList->ResourceBarrier(1, &barrier);

	unsigned width = d3d12->getWindowWidth();
	unsigned height = d3d12->getWindowHeight();
	Matrix model = Matrix::Identity;
	const Matrix& view = camera->getView();
	Matrix proj = ModuleCamera::getPerspectiveProjection(float(width) / float(height));
	Matrix mvp = model * view * proj;
	mvp = mvp.Transpose();

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.Width = float(width);
	viewport.Height = float(height);

	D3D12_RECT scissor;
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = 0;
	scissor.bottom = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE rtv = d3d12->getRenderTargetDescriptor();
	D3D12_CPU_DESCRIPTOR_HANDLE dsv = d3d12->getDepthStencilDescriptor();

	float clear[] = { 0.1f, 0.1f, 0.1f, 1.0f };
	commandList->OMSetRenderTargets(1, &rtv, false, &dsv);
	commandList->ClearRenderTargetView(rtv, clear, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissor);

	dd::xzSquareGrid(-10.0f, 10.0f, 0.0f, 1.0f, dd::colors::LightGray);
	dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 2.0f);

	debugDrawPass->record(commandList, d3d12->getWindowWidth(), d3d12->getWindowHeight(), view, proj);

	barrier = CD3DX12_RESOURCE_BARRIER::Transition(d3d12->getCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* commandLists[] = { commandList };
	d3d12->getCommandQueue()->ExecuteCommandLists(UINT(std::size(commandLists)), commandLists);

}
