//
// Created by Michael Ankele on 2025-05-06.
//

#pragma once

#include <lib/yrenderer/scene/MeshEmitter.h>


class WorldModelsEmitter : public yrenderer::MeshEmitter {
public:
	explicit WorldModelsEmitter(yrenderer::Context* ctx);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
	void emit_transparent(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
};

