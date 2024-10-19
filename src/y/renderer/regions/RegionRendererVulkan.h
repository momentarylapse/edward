/*
 * RegionRendererVulkan.h
 *
 *  Created on: 11 Oct 2023
 *      Author: michi
 */

#ifndef SRC_RENDERER_REGIONS_REGIONRENDERERVULKAN_H_
#define SRC_RENDERER_REGIONS_REGIONRENDERERVULKAN_H_

#include "RegionRendererCommon.h"
#ifdef USING_VULKAN

#include <lib/math/rect.h>

class RegionRendererVulkan : public RegionRendererCommon {
public:
	void draw(const RenderParams& params) override;
};

#endif

#endif /* SRC_RENDERER_REGIONS_REGIONRENDERERVULKAN_H_ */
