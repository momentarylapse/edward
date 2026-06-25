//
// Created by michi on 25.01.25.
//

#pragma once


#include <helper/ResourceManager.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/image/color.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
//#include <lib/xhui/Context.h>

#include "MultiView.h"

namespace polymesh {
	struct Mesh;
	struct MeshEdit;
}
class ResourceManager;
class MultiViewWindow;
namespace yrenderer {
	struct RenderViewData;
	struct RenderParams;
	struct Material;
	struct Context;
}
namespace xhui {
	class Context;
}
class Painter;
struct VisibilityFilter;

// color parameters are in LINEAR space!

class DrawingHelper {
public:
	explicit DrawingHelper(yrenderer::Context* ctx, xhui::Context* xhui_ctx);

	static void clear(const yrenderer::RenderParams& params, const color& c);

	void draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vb, yrenderer::Material* material, int pass_no = 0, const string& vertex_module = "default");

	yrenderer::Context* ctx;

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

enum class Style {
	DEFAULT,
	WARNING,
	ERROR,
};

void draw_boxed_str(Painter* p, const vec2& pos, const string& str, int align = -1, Style style = Style::DEFAULT);
void draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& a, MultiViewType kind, const base::optional<Hover>& hover, const base::set<int>& sel, const VisibilityFilter& filter);

}


Array<vec3> mesh_edit_to_lines(const polymesh::Mesh& mesh, const polymesh::MeshEdit& ed);

