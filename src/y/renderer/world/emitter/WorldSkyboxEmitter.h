//
// Created by Michael Ankele on 2025-05-07.
//

#pragma once

#include <lib/yrenderer/scene/MeshEmitter.h>

class WorldSkyboxEmitter : public yrenderer::MeshEmitter {
public:
	WorldSkyboxEmitter(yrenderer::Context* ctx);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};

