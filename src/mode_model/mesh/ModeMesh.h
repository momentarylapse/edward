//
// Created by Michael Ankele on 2025-02-20.
//

#pragma once



#include <lib/base/optional.h>
#include <view/Mode.h>
#include <view/Hover.h>
#include <view/VisibilityStack.h>
#include "../data/DataModel.h"

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
class ModeMeshGeometry;
class ModeMeshMaterial;
class ModeMeshNormals;
class ModeMeshSculpt;
class ModeMeshUV;

class ModeMesh : public SubMode {
public:
	explicit ModeMesh(ModeModel* parent);
	~ModeMesh() override;

	obs::source out_current_material_changed{this, "current-material-changed"};
	obs::source out_texture_level_changed{this, "texture-level-changed"};

	void on_enter_rec() override;
	void on_connect_events_rec() override;
	void on_leave_rec() override;
	void on_enter() override;
	void on_connect_events() override;
	void on_leave() override;
	void on_command(const string &id) override;
	void on_update_menu() override;
	void update_menu_presentation_mode();
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void draw_polygons(const yrenderer::RenderParams& params, MultiViewWindow* win);
	void draw_edges(const yrenderer::RenderParams& params, MultiViewWindow* win, const base::set<int>& sel);
	void draw_mesh(const yrenderer::RenderParams& params, MultiViewWindow* win, bool with_selection);

	void on_update_selection();
	void update_vb();
	void update_selection_vb();
	void on_update_topology(const polymesh::MeshEdit& edit);
	void update_edge_info();

	owned<ModeMeshGeometry> mode_mesh_geometry;
	owned<ModeMeshMaterial> mode_mesh_material;
	owned<ModeMeshNormals> mode_mesh_normals;
	owned<ModeMeshSculpt> mode_mesh_sculpt;
	owned<ModeMeshUV> mode_mesh_uv;

	DataModel* data;
	Array<polymesh::Edge> edges_cached;
	struct EdgeInfo {
		int polygons[2];
		int sides[2];
		vec3 normal;
	};
	Array<EdgeInfo> edge_infos;
	bool normals_dirty = false;
	int id_runner;

	int current_material;
	void set_current_material(int index);
	int current_texture_level;
	void set_current_texture_level(int index);

	void set_edit_mesh(ModelMesh* mesh);

	owned_array<ygfx::VertexBuffer> vertex_buffers;
	ygfx::VertexBuffer* vertex_buffer_physical;
	ygfx::VertexBuffer* vertex_buffer_selection;
	ygfx::VertexBuffer* vertex_buffer_hover;
	yrenderer::Material* material_physical;
	yrenderer::Material* material_selection;
	yrenderer::Material* material_hover;

	enum class PresentationMode {
		Vertices,
		Edges,
		Polygons,
		Surfaces
	} presentation_mode;
	void set_presentation_mode(PresentationMode m);

	VisibilityStack visibility_stack;

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	Selection select_in_rect(MultiViewWindow* win, const rect& r);
	void make_selection_consistent(Selection& sel) const;
	base::optional<Box> get_selection_box(const Selection& sel) const;
};


