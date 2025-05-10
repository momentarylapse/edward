//
// Created by Michael Ankele on 2025-05-08.
//

#pragma once

#include "../../scene/MeshEmitter.h"


class WorldInstancedEmitter : public MeshEmitter {
public:
	WorldInstancedEmitter();
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};
