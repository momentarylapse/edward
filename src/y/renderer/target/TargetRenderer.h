/*
 * TargetRenderer.h
 *
 *  Created on: Nov 23, 2021
 *      Author: michi
 */

#pragma once


#include "../Renderer.h"

class TargetRenderer : public Renderer {
public:
	explicit TargetRenderer(const string &name);

	void draw(const RenderParams& params) override;

	int ch_end;
};
