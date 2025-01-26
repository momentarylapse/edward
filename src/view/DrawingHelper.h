//
// Created by michi on 25.01.25.
//

#ifndef DRAWINGHELPER_H
#define DRAWINGHELPER_H



#include <y/graphics-fwd.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/xhui/ContextVulkan.h>

class MultiViewWindow;

class DrawingHelper {
public:
	explicit DrawingHelper(xhui::ContextVulkan* ctx);
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	void draw_lines(const Array<vec3>& points, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

	xhui::ContextVulkan* context;
	MultiViewWindow* window;
	void set_window(MultiViewWindow* win);

	Shader* shader = nullptr;
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
};



#endif //DRAWINGHELPER_H
