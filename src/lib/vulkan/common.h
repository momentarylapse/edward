/*
 * common.h
 *
 *  Created on: 17.09.2022
 *      Author: michi
 */
#pragma once

#if HAS_LIB_VULKAN

#include <lib/base/set.h>

namespace vulkan {

extern int verbosity;

enum class Feature {
	ANISOTROPY,
	SWAP_CHAIN,
	PRESENT,
	GRAPHICS,
	COMPUTE,
	VALIDATION,
	RTX,
	MESH_SHADER,
	GEOMETRY_SHADER,
	TESSELATION_SHADER,
	MULTISAMPLE,
	PHYSICAL,
	_NUM
};

struct Requirements {
	base::set<Feature> required;
	base::set<Feature> optional;
};


}


#endif
