//
// Created by Michael Ankele on 2025-02-20.
//

#ifndef MODEMESH_H
#define MODEMESH_H



#include <lib/base/optional.h>
#include "../../view/Mode.h"
#include "../../view/Hover.h"
#include "../data/DataModel.h"

class MultiViewWindow;
namespace xhui {
	class Panel;
}
class Material;
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
	void on_prepare_scene(const RenderParams& params) override;
	void on_draw_background(const RenderParams& params, RenderViewData& rvd) override;
	void on_draw_win(const RenderParams& params, MultiViewWindow* win) override;
	void on_draw_post(Painter*) override;
	void on_command(const string& id) override;
	void on_key_down(int key) override;
	void on_mouse_move(const vec2& m, const vec2& d) override;

	void on_update_selection();
	void update_vb();
	void update_selection_vb();

	void optimize_view();

	void copy();
	void paste();

	owned<ModeMeshMaterial> mode_mesh_material;
	owned<ModeMeshSculpt> mode_mesh_sculpt;

	DataModel* data;

	int current_material;
	void set_current_material(int index);
	int current_texture_level;
	void set_current_texture_level(int index);

	void set_edit_mesh(ModelMesh* mesh);

	owned<ModelMesh> temp_mesh;
	owned_array<VertexBuffer> vertex_buffers;
	VertexBuffer* vertex_buffer_physical;
	VertexBuffer* vertex_buffer_selection;
	VertexBuffer* vertex_buffer_hover;
	owned_array<Material> materials;
	Material* material_physical;
	Material* material_selection;
	Material* material_hover;
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
