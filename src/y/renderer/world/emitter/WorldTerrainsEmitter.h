//
// Created by Michael Ankele on 2025-05-06.
//

#pragma once

#include "../../scene/MeshEmitter.h"

class WorldTerrainsEmitter : public MeshEmitter {
public:
	WorldTerrainsEmitter();
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};


