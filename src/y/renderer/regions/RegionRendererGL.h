/*
 * RegionRendererGL.h
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#ifndef SRC_RENDERER_REGIONS_REGIONRENDERERGL_H_
#define SRC_RENDERER_REGIONS_REGIONRENDERERGL_H_

#include "RegionRendererCommon.h"
#ifdef USING_OPENGL

#include <lib/math/rect.h>

class RegionRendererGL : public RegionRendererCommon {
public:
	void draw(const RenderParams& params) override;
};

#endif

#endif /* SRC_RENDERER_REGIONS_REGIONRENDERERGL_H_ */
