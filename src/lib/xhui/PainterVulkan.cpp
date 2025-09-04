#if HAS_LIB_VULKAN

#include "Painter.h"
#include "Context.h"
#include "Theme.h"
#include <lib/ygraphics/Context.h>
#include "../vulkan/vulkan.h"
#include "../image/image.h"
#include "../math/mat4.h"


using namespace vulkan;


namespace xhui {

struct Parameters {
	mat4 matrix;
	color col;
	vec2 size;
	float radius, softness;
};


void Painter::draw_ximage(const rect& r, const XImage* image) {
	auto dset = aux->get_descriptor_set(image->texture.get());
	Parameters params;
	params.matrix = mat_pixel_to_rel * mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1);
	params.col = _color;
	params.size = {r.width(),r.height()};
	params.radius = 0;
	params.softness = 0;

	cb->bind_pipeline(aux->pipeline_alpha);
	cb->push_constant(0, sizeof(params), &params);

	cb->bind_descriptor_set(0, dset);
	cb->draw(aux->vb);
}


}

#endif
