#include "ModeMeshUV.h"
#include "../ModeMesh.h"
#include "../../data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/os/msg.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/EdwardWindow.h>
#include <view/MultiView.h>

#include "action/ActionModelUVMoveSelection.h"
#include "lib/base/iter.h"
#include "lib/yrenderer/MaterialManager.h"
#include "mode_model/ModeModel.h"
#include "view/ActionController.h"


class MeshUVOpButtons : public xhui::Panel {
public:
	explicit MeshUVOpButtons(MultiView* multi_view) : xhui::Panel("mesh-uv-op-buttons") {
		from_source(R"foodelim(
Dialog mesh-uv-op-buttons '' propagateevents
	Grid ? '' spacing=20 vertical
		Button mouse-action 'T' 'tooltip=Left button action: move selection' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");

		event("mouse-action", [this, multi_view] {
			auto ac = multi_view->action_controller.get();
			const auto mode = ac->action_mode();
			if (mode == MouseActionMode::MOVE) {
				ac->set_action_mode(MouseActionMode::ROTATE);
				set_options("mouse-action", "image=rf-rotate");
			} else if (mode == MouseActionMode::ROTATE) {
				ac->set_action_mode(MouseActionMode::SCALE);
				set_options("mouse-action", "image=rf-scale");
			} else if (mode == MouseActionMode::SCALE) {
				ac->set_action_mode(MouseActionMode::MOVE);
				set_options("mouse-action", "image=rf-translate");
			}
			set_string("mouse-action", multi_view->action_controller->action_name().sub(0, 1).upper());
		});
	}
};

ModeMeshUV::ModeMeshUV(ModeMesh* parent) :
	SubMode(parent)
{
	mode_mesh = parent;
	auto mode_model = static_cast<ModeModel*>(mode_mesh->get_parent());
	multi_view = mode_model->multi_view_2d.get();
	generic_data = mode_mesh->generic_data;
}

void ModeMeshUV::on_enter_rec() {
	auto mode_model = static_cast<ModeModel*>(mode_mesh->get_parent());
	mode_model->multi_view_panel->set_multi_view(multi_view);

	if (!vb_bg) {
		vb_bg = new ygfx::VertexBuffer("3f,3f,2f");
		const float R = 100;
		vb_bg->create_quad({-R,R, -R,R}, {-R,R, -R,R});
	}
	if (!material_bg) {
		material_bg = session->resource_manager->material_manager->create_internal();
		material_bg->albedo = Black;
		material_bg->emission = White;
		material_bg->pass0.shader_path = "default-pure-emission.shader";
		material_bg->pass0.cull_mode = ygfx::CullMode::NONE;
		material_bg->pass0.z_test = false;
		material_bg->pass0.z_write = false;
	}
	material_bg->textures[0] = mode_mesh->data->materials[0]->textures[0];

	mode_mesh->data->out_changed >> create_sink([this] {
		update_uvs();
	});
}

void ModeMeshUV::on_leave_rec() {
	mode_mesh->data->unsubscribe(this);
	auto mode_model = static_cast<ModeModel*>(mode_mesh->get_parent());
	mode_model->multi_view_panel->set_multi_view(mode_mesh->multi_view);
}

void ModeMeshUV::on_enter() {
	multi_view->set_allow_select(true);
	multi_view->set_allow_action(true);
	multi_view->set_show_grid(true);
	multi_view->data_sets = {
		{MultiViewType::MODEL_SKIN_VERTEX, &uv}
	};

	multi_view->f_hover = [this] (MultiViewWindow* win, const vec2& m) {
		return get_hover(win, m);
	};
	multi_view->f_select = [this] (MultiViewWindow* win, const rect& r) {
		return select_in_rect(win, r);
	};
	multi_view->f_get_selection_box = [this] (const Selection& sel) {
		return get_selection_box(sel);
	};
	/*multi_view->f_make_selection_consistent = [this] (Selection& sel) {
		-> select mesh vertices etc...
		return make_selection_consistent(sel);
	};*/
	multi_view->f_create_action = [this] {
		return new ActionModelUVMoveSelection(mode_mesh->data->editing_mesh, multi_view->selection);
	};
	multi_view->out_selection_changed >> create_sink([this] {
		export_selection();
		out_redraw();
	});

	set_overlay_panel(new MeshUVOpButtons(multi_view));

	// import
	update_uvs();
	import_selection();
}

void ModeMeshUV::update_uvs() {
	uv.clear();
	for (const auto& p : mode_mesh->data->editing_mesh->polygons)
		for (int i=0; i<p.side.num; i++)
			uv.add({p.side[i].skin_vertex[0]});
}

void ModeMeshUV::import_selection() {
	auto& sel = multi_view->selection[MultiViewType::MODEL_SKIN_VERTEX];
	sel.clear();
	const auto& selp = mode_mesh->multi_view->selection[MultiViewType::MODEL_POLYGON];
	int n0 = 0;
	for (auto&& [i, p]: enumerate(mode_mesh->data->editing_mesh->polygons)) {
		if (selp.contains(i)) {
			for (int k=0; k<p.side.num; k++)
				sel.add(n0 + k);
		}
		n0 += p.side.num;
	}
	multi_view->update_selection_box();
}

void ModeMeshUV::export_selection() {
	const auto& sel = multi_view->selection[MultiViewType::MODEL_SKIN_VERTEX];
	auto& selv = mode_mesh->multi_view->selection[MultiViewType::MODEL_VERTEX];

}

void ModeMeshUV::on_draw_background(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd) {
	mode_mesh->on_draw_background(params, rvd);

	auto dh = session->drawing_helper;
	dh->draw_mesh(params, rvd, mat4::ID, vb_bg.get(), material_bg);
}

void ModeMeshUV::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {

	Array<vec3> points;
	for (const auto& poly : mode_mesh->data->editing_mesh->polygons)
		for (int i=0; i<poly.side.num; i++) {
			points.add(poly.side[i].skin_vertex[0]);
			points.add(poly.side[(i+1) % poly.side.num].skin_vertex[0]);
		}
	session->drawing_helper->set_z_test(false);
	session->drawing_helper->set_line_width(2.0f);
	session->drawing_helper->set_color(White);
	session->drawing_helper->draw_lines(points, false);

	points.clear();
	const auto& sel = multi_view->selection[MultiViewType::MODEL_SKIN_VERTEX];
	int n0 = 0;
	for (const auto& poly : mode_mesh->data->editing_mesh->polygons) {
		for (int i=0; i<poly.side.num; i++)
			if (sel.contains(n0 + i) and sel.contains(n0 + ((i+1) % poly.side.num))) {
				points.add(poly.side[i].skin_vertex[0]);
				points.add(poly.side[(i+1) % poly.side.num].skin_vertex[0]);
			}
		n0 += poly.side.num;
	}
	session->drawing_helper->set_line_width(3.0f);
	session->drawing_helper->set_color(Red);
	session->drawing_helper->draw_lines(points, false);
	session->drawing_helper->set_z_test(true);
}

void ModeMeshUV::on_draw_post(Painter* p) {
	//mode_mesh->on_draw_post(p);

	drawing2d::draw_data_points(p, multi_view->active_window, uv, MultiViewType::MODEL_SKIN_VERTEX, multi_view->hover, multi_view->selection[MultiViewType::MODEL_SKIN_VERTEX]);

	draw_info(p, "uv...");
}

base::optional<Hover> ModeMeshUV::get_hover(MultiViewWindow* win, const vec2& m) const {
	base::optional<Hover> h;

	//float zmin = multi_view->view_port.radius * 2;
	for (const auto& [i, v]: enumerate(uv)) {
		const auto pp = win->project(v.pos);
		if (pp.z <= 0 or pp.z >= 1)
			continue;
		if ((pp.xy() - m).length_fuzzy() > 10)
			continue;
		h = {MultiViewType::MODEL_SKIN_VERTEX, i, v.pos};
	}

	return h;
}

Selection ModeMeshUV::select_in_rect(MultiViewWindow* win, const rect& r) {
	Selection sel;
	sel.add({MultiViewType::MODEL_SKIN_VERTEX, {}});
	sel[MultiViewType::MODEL_SKIN_VERTEX] = MultiView::select_points_in_rect(win, r, uv);
	return sel;
}

base::optional<Box> ModeMeshUV::get_selection_box(const Selection& sel) const {
	return MultiView::points_get_selection_box(uv, sel[MultiViewType::MODEL_SKIN_VERTEX]);
}
