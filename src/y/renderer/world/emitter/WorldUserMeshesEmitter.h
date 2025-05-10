//
// Created by Michael Ankele on 2025-05-08.
//

#pragma once

#include "../../scene/MeshEmitter.h"


class WorldUserMeshesEmitter : public MeshEmitter {
public:
	WorldUserMeshesEmitter();
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
	void emit_transparent(const RenderParams& params, RenderViewData& rvd) override;
};

