//
// Created by Michael Ankele on 2025-02-20.
//

#ifndef MODEMESH_H
#define MODEMESH_H



#include <lib/base/optional.h>
#include "../../view/Mode.h"
#include "../../view/Hover.h"
#include "../data/DataModel.h"

struct Edge;
class MultiViewWindow;
namespace xhui {
	class Panel;
}
namespace yrenderer {
	class Material;
}
class ModeModel;
class ModeMeshMaterial;
class ModeMeshSculpt;

class ModeMesh : public SubMode {
public:
	explicit ModeMesh(ModeModel* parent);
	~ModeMesh() override;

	obs::source out_current_material_changed{this, "current-material-changed"};
	obs::source out_texture_level_changed{this, "texture-level-changed"};

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_enter() override;
	void on_leave() override;
	void on_prepare_scene(const yrenderer::RenderParams& params) override;
	void on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) override;
	void on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) override;
	void draw_polygons(const yrenderer::RenderParams& params, MultiViewWindow* win);
	void draw_edges(const yrenderer::RenderParams& params, MultiViewWindow* win, const base::set<int>& sel);
	void on_draw_post(Painter*) override;
	void on_command(const string& id) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	void on_update_selection();
	void update_vb();
	void update_selection_vb();
	void on_update_topology();
	void update_edge_info();

	void optimize_view();

	void copy();
	void paste();

	owned<ModeMeshMaterial> mode_mesh_material;
	owned<ModeMeshSculpt> mode_mesh_sculpt;

	DataModel* data;
	Array<Edge> edges_cached;
	struct EdgeInfo {
		int polygons[2];
		int sides[2];
		vec3 normal;
	};
	Array<EdgeInfo> edge_infos;
	bool normals_dirty = false;

	int current_material;
	void set_current_material(int index);
	int current_texture_level;
	void set_current_texture_level(int index);

	void set_edit_mesh(ModelMesh* mesh);

	owned<ModelMesh> temp_mesh;
	owned_array<ygfx::VertexBuffer> vertex_buffers;
	ygfx::VertexBuffer* vertex_buffer_physical;
	ygfx::VertexBuffer* vertex_buffer_selection;
	ygfx::VertexBuffer* vertex_buffer_hover;
	owned_array<yrenderer::Material> materials;
	yrenderer::Material* material_physical;
	yrenderer::Material* material_selection;
	yrenderer::Material* material_hover;
	Array<int> event_ids_rec;
	Array<int> event_ids;

	enum class PresentationMode {
		Vertices,
		Edges,
		Polygons,
		Surfaces
	} presentation_mode;
	void set_presentation_mode(PresentationMode m);
	void update_menu();

	base::optional<Hover> get_hover(MultiViewWindow* win, const vec2& m) const;
	Data::Selection select_in_rect(MultiViewWindow* win, const rect& r);
	void make_selection_consistent(Data::Selection& sel) const;
	base::optional<Box> get_selection_box(const Data::Selection& sel) const;
};



#endif //MODEMESH_H
