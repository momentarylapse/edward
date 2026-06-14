//
// Created by Michael Ankele on 2025-02-20.
//

#pragma once



#include <lib/base/optional.h>
#include <view/Mode.h>
#include <view/Hover.h>
#include "../../data/DataModel.h"

namespace polymesh {
	struct Edge;
}
class MultiViewWindow;
namespace xhui {
	class Panel;
}
namespace yrenderer {
	struct Material;
}
class ModeModel;
class ModeMesh;
class ModeMeshMaterial;
class ModeMeshSculpt;
class ModeMeshUV;

class ModeMeshGeometry : public SubMode {
public:
	explicit ModeMeshGeometry(ModeMesh* parent);
	~ModeMeshGeometry() override;

	void on_enter_rec() override;
	void on_connect_events_rec() override;
	void on_leave_rec() override;
	void on_enter() override;
	void on_connect_events() override;
	void on_leave() override;
	void on_update_menu() override;
	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_command(const string& id) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	void on_update_selection();
	void on_update_topology();
	void update_edge_info();

	void optimize_view();

	void copy();
	void paste();

	ModeMesh* mode_mesh;
	DataModel* data;
	Array<polymesh::Edge> edges_cached;
	struct EdgeInfo {
		int polygons[2];
		int sides[2];
		vec3 normal;
	};
	Array<EdgeInfo> edge_infos;
	bool normals_dirty = false;

	owned<ModelMesh> temp_mesh;

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	Selection select_in_rect(MultiViewWindow* win, const rect& r);
	void make_selection_consistent(Selection& sel) const;
	base::optional<Box> get_selection_box(const Selection& sel) const;
};


