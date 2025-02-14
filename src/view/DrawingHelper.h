//
// Created by michi on 25.01.25.
//

#ifndef DRAWINGHELPER_H
#define DRAWINGHELPER_H


#include <helper/ResourceManager.h>
#include <y/graphics-fwd.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/xhui/ContextVulkan.h>

class ResourceManager;
class MultiViewWindow;
class Material;

class DrawingHelper {
public:
#ifdef USING_VULKAN
	explicit DrawingHelper(xhui::ContextVulkan* ctx, ResourceManager* rm);
#else
	explicit DrawingHelper(ResourceManager* rm);
#endif
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	void draw_lines(const Array<vec3>& points, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

#ifdef USING_VULKAN
	xhui::ContextVulkan* context;
#endif
	ResourceManager* resource_manager;
	MultiViewWindow* window;
	void set_window(MultiViewWindow* win);

	Shader* shader = nullptr;
#ifdef USING_VULKAN
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
#endif

	Material* material_hover;
	Material* material_selection;
};



#endif //DRAWINGHELPER_H
