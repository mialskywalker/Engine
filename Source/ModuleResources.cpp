#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ModuleResources.h"
#include <vector>

#include "DirectXTex.h"

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

ComPtr<ID3D12Resource> ModuleResources::createTextureFromFile(const std::filesystem::path& filePath)
{

	D3D12Module* d3d12 = app->getD3D12();

	const wchar_t* path = filePath.c_str();

	DirectX::ScratchImage image;
	if (FAILED(LoadFromDDSFile(path, DDS_FLAGS_NONE, nullptr, image)))
	{
		if (FAILED(LoadFromTGAFile(path, nullptr, image)))
		{
			LoadFromWICFile(path, WIC_FLAGS_NONE, nullptr, image);
		}
	}
	
	ComPtr<ID3D12Resource> texture;
	const DirectX::TexMetadata metaData = image.GetMetadata();
	D3D12_RESOURCE_DESC textureDesc;
	CD3DX12_HEAP_PROPERTIES textureHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	UINT64 size;
	ComPtr<ID3D12Resource> stagingBuffer;
	D3D12_RESOURCE_DESC stagingDesc;
	CD3DX12_HEAP_PROPERTIES stagingProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	if (metaData.mipLevels == 1)
	{
		DirectX::ScratchImage mipImage;
		HRESULT hr = GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipImage);
		const DirectX::TexMetadata mipMetaData = mipImage.GetMetadata();
		textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(mipMetaData.format, UINT64(mipMetaData.width), UINT(mipMetaData.height), UINT16(mipMetaData.arraySize), UINT16(mipMetaData.mipLevels));

		d3d12->getDevice()->CreateCommittedResource(&textureHeap, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

		size = GetRequiredIntermediateSize(texture.Get(), 0, mipImage.GetImageCount());
		
		stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		d3d12->getDevice()->CreateCommittedResource(&stagingProps, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));

		std::vector<D3D12_SUBRESOURCE_DATA> subData;
		subData.reserve(size);

		for (size_t item = 0; item < mipMetaData.arraySize; ++item)
		{
			for (size_t level = 0; level < mipMetaData.mipLevels; ++level)
			{
				const DirectX::Image* subImg = mipImage.GetImage(level, item, 0);
				D3D12_SUBRESOURCE_DATA data = { subImg->pixels, subImg->rowPitch, subImg->slicePitch };
				subData.push_back(data);
			}
		}

		UpdateSubresources(commandList.Get(), texture.Get(), stagingBuffer.Get(), 0, 0, UINT(mipImage.GetImageCount()), subData.data());
	}
	else
	{
		textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(metaData.format, UINT64(metaData.width), UINT(metaData.height), UINT16(metaData.arraySize), UINT16(metaData.mipLevels));
		d3d12->getDevice()->CreateCommittedResource(&textureHeap, D3D12_HEAP_FLAG_NONE, &textureDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

		size = GetRequiredIntermediateSize(texture.Get(), 0, image.GetImageCount());

		stagingDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		d3d12->getDevice()->CreateCommittedResource(&stagingProps, D3D12_HEAP_FLAG_NONE, &stagingDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&stagingBuffer));

		std::vector<D3D12_SUBRESOURCE_DATA> subData;
		subData.reserve(size);

		for (size_t item = 0; item < metaData.arraySize; ++item)
		{
			for (size_t level = 0; level < metaData.mipLevels; ++level)
			{
				const DirectX::Image* subImg = image.GetImage(level, item, 0);
				D3D12_SUBRESOURCE_DATA data = { subImg->pixels, subImg->rowPitch, subImg->slicePitch };
				subData.push_back(data);
			}
		}

		UpdateSubresources(commandList.Get(), texture.Get(), stagingBuffer.Get(), 0, 0, UINT(image.GetImageCount()), subData.data());
	}
	
	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);

	commandList->Close();

	ID3D12CommandList* lists[] = { commandList.Get() };
	d3d12->getCommandQueue()->ExecuteCommandLists(1, lists);

	d3d12->flush();
	commandAllocator->Reset();
	commandList->Reset(commandAllocator.Get(), nullptr);

	return texture;
}

ComPtr<ID3D12Resource> ModuleResources::createRenderTarget(DXGI_FORMAT format, unsigned width, unsigned height, const float clearColor[4])
{
	ComPtr<ID3D12Resource> texture;
	//D3D12_RESOURCE_DESC textureDesc;
	CD3DX12_HEAP_PROPERTIES textureHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, UINT64(width), UINT(height), UINT16(1), UINT16(1), D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		UINT64(width),
		UINT(height),
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
	);

	D3D12_CLEAR_VALUE clear = {};
	clear.Format = format;
	clear.Color[0] = clearColor[0];
	clear.Color[1] = clearColor[1];
	clear.Color[2] = clearColor[2];
	clear.Color[3] = clearColor[3];

	app->getD3D12()->getDevice()->CreateCommittedResource(&textureHeap, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &texDesc, D3D12_RESOURCE_STATE_COMMON, &clear, IID_PPV_ARGS(&texture));

	return texture;
}

ComPtr<ID3D12Resource> ModuleResources::createDepthStencil(DXGI_FORMAT format, unsigned width, unsigned height, float clearValue)
{
	ComPtr<ID3D12Resource> texture;
	//D3D12_RESOURCE_DESC textureDesc;
	CD3DX12_HEAP_PROPERTIES textureHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	//textureDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, UINT64(width), UINT(height), UINT16(1), UINT16(1), D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	auto texDesc = CD3DX12_RESOURCE_DESC::Tex2D(
		format,
		UINT64(width),
		UINT(height),
		1, 1,
		1, 0,
		D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL
	);

	D3D12_CLEAR_VALUE clear = {};
	clear.Format = format;
	clear.DepthStencil.Depth = clearValue;

	app->getD3D12()->getDevice()->CreateCommittedResource(&textureHeap, D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES, &texDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clear, IID_PPV_ARGS(&texture));

	return texture;
}
