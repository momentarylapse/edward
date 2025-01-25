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
	void draw_lines(const Array<vec3>& points, float width);
	color _color;

	xhui::ContextVulkan* context;
	MultiViewWindow* window;
	void set_window(MultiViewWindow* win);

	Shader* shader = nullptr;
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
};



#endif //DRAWINGHELPER_H
