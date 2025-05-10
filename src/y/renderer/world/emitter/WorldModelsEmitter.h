//
// Created by Michael Ankele on 2025-05-06.
//

#pragma once

#include "../../scene/MeshEmitter.h"


class WorldModelsEmitter : public MeshEmitter {
public:
	WorldModelsEmitter();
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
	void emit_transparent(const RenderParams& params, RenderViewData& rvd) override;
};

