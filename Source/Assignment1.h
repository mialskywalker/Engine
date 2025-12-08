#pragma once

#include "Module.h"
#include "DebugDrawPass.h"

class Assignment1 : public Module
{
	std::unique_ptr<DebugDrawPass> debugDrawPass;

public:

	bool init() override;
	void render() override;

};