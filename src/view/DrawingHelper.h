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
#include <lib/xhui/Context.h>

#include "MultiView.h"

struct RenderViewData;
struct RenderParams;
class ResourceManager;
class MultiViewWindow;
class Material;
class Painter;

class DrawingHelper {
public:
	explicit DrawingHelper(xhui::Context* ctx, ResourceManager* rm);
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	void clear(const RenderParams& params, const color& c);

	void draw_lines(const Array<vec3>& points, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

	void draw_mesh(const RenderParams& params, RenderViewData& rvd, const mat4& matrix, VertexBuffer* vb, Material* material, int pass_no = 0, const string& vertex_module = "default");

	void draw_boxed_str(Painter* p, const vec2& pos, const string& str, int align = -1);

	void draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& a, MultiViewType kind, const base::optional<Hover>& hover);

	xhui::Context* context;
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
	Material* material_creation;
};



#endif //DRAWINGHELPER_H
