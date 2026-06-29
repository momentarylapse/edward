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
	void set_z_test(bool enabled);
	void set_blending(bool enabled);
	void set_color(const color& c);
	void set_line_width(float w);

	void draw_lines(const Array<vec3>& points, bool contiguous);
	void draw_lines_colored(const Array<vec3>& points, const Array<color>& col, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

	color _color;
	float line_width = 1;
	float ui_scale = 1;
	bool use_z = true;
	bool use_blending = false;
	rect area;
	mat4 mat;
	mat4 dummy_projection;

	ygfx::DrawingHelperData* aux = nullptr;
	RenderViewData* rvd = nullptr;
	void* render_pass = nullptr;
};

} // namespace yrenderer
