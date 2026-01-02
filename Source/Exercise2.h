#pragma once

#include "Module.h"

class Exercise2 : public Module
{
	ComPtr<ID3D12Resource> vertexBufferView;

public:
	bool init() override;
};