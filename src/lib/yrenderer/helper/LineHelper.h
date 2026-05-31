#pragma once

#include <lib/image/color.h>
#include <lib/math/rect.h>
#include <lib/math/mat4.h>

struct vec3;

namespace ygfx {
	struct DrawingHelperData;
}

namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;

class LineHelper {
public:
	void start_frame(float ui_scale=1);
	void begin_draw(const RenderParams& params, RenderViewData& rvd);
	void set_z(bool enabled);
	void set_color(const color& c);
	void set_line_width(float w);

	void draw_lines(const Array<vec3>& points, bool contiguous);

	color _color;
	float line_width = 1;
	float ui_scale = 1;
	bool use_z = true;
	rect area;
	mat4 mat;

	ygfx::DrawingHelperData* aux = nullptr;
	RenderViewData* rvd = nullptr;
	void* render_pass = nullptr;
};

} // namespace yrenderer
