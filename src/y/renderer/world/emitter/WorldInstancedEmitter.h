//
// Created by Michael Ankele on 2025-05-08.
//

#pragma once

#include <lib/yrenderer/scene/MeshEmitter.h>


class WorldInstancedEmitter : public yrenderer::MeshEmitter {
public:
	explicit WorldInstancedEmitter(yrenderer::Context* ctx);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};
