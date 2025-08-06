/*
 * TargetRenderer.h
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#pragma once


#include <lib/yrenderer/Renderer.h>

namespace yrenderer {

class TargetRenderer : public Renderer {
public:
	explicit TargetRenderer(Context* ctx, const string &name);

	void draw(const RenderParams& params) override;

	int ch_end;
};

}
