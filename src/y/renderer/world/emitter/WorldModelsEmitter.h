//
// Created by Michael Ankele on 2025-05-06.
//

#pragma once

#include <lib/yrenderer/scene/MeshEmitter.h>


class WorldOpaqueModelsEmitter : public yrenderer::MeshEmitter {
public:
	explicit WorldOpaqueModelsEmitter(yrenderer::Context* ctx);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};

class WorldTransparentModelsEmitter : public yrenderer::MeshEmitter {
public:
	explicit WorldTransparentModelsEmitter(yrenderer::Context* ctx);
	void emit(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, bool shadow_pass) override;
};

