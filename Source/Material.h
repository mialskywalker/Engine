#pragma once

namespace tinygltf { class Model; struct Material; }

class Material
{
	struct MaterialData
	{
		Vector4 baseColor;
		BOOL hasColorTexture;
	};

	MaterialData data;
	ComPtr<ID3D12Resource> colorTexture;
	unsigned index = 0;

	ComPtr<ID3D12Resource> materialBuffer;
	D3D12_GPU_VIRTUAL_ADDRESS materialAddress;

public:
	Material();
	~Material();

	void load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath);

	const D3D12_GPU_VIRTUAL_ADDRESS getMatGPUAddress() { return materialAddress; }

	ID3D12Resource* getColorTexture() { return colorTexture.Get(); }

	const unsigned& getIndex() const { return this->index; }
};