#include "Globals.h"
#include "Material.h"
#include "utils_gltf.h"
#include "Application.h"
#include "ModuleResources.h"
#include "ModuleShaderDescriptors.h"

Material::Material() {}

Material::~Material() {}

void Material::load(const tinygltf::Model& model, const tinygltf::Material& material, const char* basePath)
{
	/*data.baseColor = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]),
		float(material.pbrMetallicRoughness.baseColorFactor[1]),
		float(material.pbrMetallicRoughness.baseColorFactor[2]),
		float(material.pbrMetallicRoughness.baseColorFactor[3]));*/

	phong.diffuseColor = Vector4(float(material.pbrMetallicRoughness.baseColorFactor[0]),
		float(material.pbrMetallicRoughness.baseColorFactor[1]),
		float(material.pbrMetallicRoughness.baseColorFactor[2]),
		float(material.pbrMetallicRoughness.baseColorFactor[3]));

	phong.Kd = 1.0f;
	phong.Ks = 0.3f;
	phong.shininess = 16.0f;
	//data.hasColorTexture = FALSE;
	phong.hasDiffuseTex = FALSE;

	if (material.pbrMetallicRoughness.baseColorTexture.index >= 0)
	{
		const tinygltf::Texture& texture = model.textures[material.pbrMetallicRoughness.baseColorTexture.index];
		const tinygltf::Image& image = model.images[texture.source];
		if (!image.uri.empty())
		{
			colorTexture = app->getResources()->createTextureFromFile(std::string(basePath) + image.uri);
			//data.hasColorTexture = TRUE;
			phong.hasDiffuseTex = TRUE;
		}
	}

	/*if (data.hasColorTexture)
		index = app->getDescriptors()->createSRV(colorTexture.Get());
	else
		index = app->getDescriptors()->createNullSRV();*/
	if (phong.hasDiffuseTex)
		index = app->getDescriptors()->createSRV(colorTexture.Get());
	else
		index = app->getDescriptors()->createNullSRV();

	//materialBuffer = app->getResources()->createDefaultBuffer(alignUp(sizeof(MaterialData), D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT), &data);
	//materialAddress = materialBuffer->GetGPUVirtualAddress();
}

void Material::setPhong(float kd, float ks, float shiny)
{
	phong.Kd = kd;
	phong.Ks = ks;
	phong.shininess = shiny;
}
