#include "CubeMapSource.h"
#include <Config.h>
#include <graphics-impl.h>

const kaba::Class* CubeMapSource::_class = nullptr;;

CubeMapSource::CubeMapSource() {
	min_depth = 1.0f;
	max_depth = 100000.0f;
	resolution = config.get_int("cubemap.resolution", 128);
	update_rate = config.get_int("cubemap.update_rate", 9);
	counter = 0;
}

CubeMapSource::~CubeMapSource() = default;

