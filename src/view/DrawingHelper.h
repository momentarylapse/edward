//
// Created by michi on 25.01.25.
//

#ifndef DRAWINGHELPER_H
#define DRAWINGHELPER_H


#include <helper/ResourceManager.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
//#include <lib/xhui/Context.h>

#include "MultiView.h"

struct MeshEdit;
struct PolygonMesh;
class ResourceManager;
class MultiViewWindow;
namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;
	class Material;
	class Context;
}
namespace xhui {
	class Context;
}
class Painter;

class DrawingHelper {
public:
	explicit DrawingHelper(yrenderer::Context* ctx, xhui::Context* xhui_ctx);
	void set_color(const color& color);
	color _color;

	void set_line_width(float width);
	float _line_width;

	static void clear(const yrenderer::RenderParams& params, const color& c);

	void draw_lines(const Array<vec3>& points, bool contiguous = true);
	void draw_lines_colored(const Array<vec3>& points, const Array<color>& col, bool contiguous = true);
	void draw_circle(const vec3& center, const vec3& axis, float r);

	void draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vb, yrenderer::Material* material, int pass_no = 0, const string& vertex_module = "default");

	yrenderer::Context* ctx;
	xhui::Context* xhui_ctx;
	MultiViewWindow* window;
	void set_window(MultiViewWindow* win);
	void set_blending(bool b);
	bool _blending = false;
	void set_z_test(bool b);
	bool z_test = true;

	ygfx::Shader* shader = nullptr;
#ifdef USING_VULKAN
	vulkan::GraphicsPipeline* pipeline = nullptr;
	vulkan::GraphicsPipeline* pipeline_alpha = nullptr;
	vulkan::GraphicsPipeline* pipeline_no_z_test = nullptr;
	vulkan::DescriptorSet* dset = nullptr;
#endif

	yrenderer::Material* material_hover;
	yrenderer::Material* material_selection;
	yrenderer::Material* material_creation;
	yrenderer::Material* material_shadow;

	static const float LINE_THIN;
	static const float LINE_MEDIUM;
	static const float LINE_THICK;
	static const float LINE_EXTRA_THICK;
	static const color COLOR_X;
};

namespace drawing2d {

void draw_boxed_str(Painter* p, const vec2& pos, const string& str, int align = -1);
void draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& a, MultiViewType kind, const base::optional<Hover>& hover, const base::set<int>& sel);

}


Array<vec3> mesh_edit_to_lines(const PolygonMesh& mesh, const MeshEdit& ed);


#endif //DRAWINGHELPER_H
