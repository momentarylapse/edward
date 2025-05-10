//
// Created by Michael Ankele on 2025-05-07.
//

#pragma once

#include "../../scene/MeshEmitter.h"

class WorldSkyboxEmitter : public MeshEmitter {
public:
	WorldSkyboxEmitter();
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};

