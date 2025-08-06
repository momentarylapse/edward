//
// Created by Michael Ankele on 2025-05-05.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/pointer.h>

namespace yrenderer {

class Context;
struct RenderViewData;
struct RenderParams;


class MeshEmitter : public Sharable<VirtualBase> {
public:
	explicit MeshEmitter(Context* ctx, const string& name);

	virtual void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {}
	virtual void emit_transparent(const RenderParams& params, RenderViewData& rvd) {}

	Context* ctx;
	int channel;
};

}

