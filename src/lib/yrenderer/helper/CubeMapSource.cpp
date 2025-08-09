#include "CubeMapSource.h"
#include "../scene/CameraParams.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/scene/SceneRenderer.h>

namespace yrenderer {

int cubemap_default_resolution = 256;
int cubemap_default_rate = 9;

CubeMapSource::CubeMapSource() {
	pos = vec3(0, 0, 0);
	min_depth = 1.0f;
	max_depth = 100000.0f;
	resolution = cubemap_default_resolution;
	update_rate = cubemap_default_rate;
	counter = 0;
}

CubeMapSource::~CubeMapSource() = default;

}



