/*
 * ModeModelMesh.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESH_H_
#define MODEMODELMESH_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
namespace nix {
	class VertexBuffer;
};
class ModeModel;
class MeshSelectionMode;
class ModeModelMeshTexture;
class ModeModelMeshMaterial;
class ModeModelMeshDeform;
class ModeModelMeshPaint;

class ModeModelMesh: public Mode<ModeModel, DataModel> {
public:
	ModeModelMesh(ModeModel *parent, MultiView::MultiView *mv3, MultiView::MultiView *mv2);
	virtual ~ModeModelMesh();

	class State : public obs::Node<VirtualBase> {
	public:
		obs::source out_current_material_changed{this, "current-material-changed"};
		obs::source out_current_skin_changed{this, "current-skin-changed"};
		~State(){}
	} state;

	void on_start() override;
	void on_enter() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_data_update();
	void on_update_menu() override;
	void on_selection_change() override;
	void on_view_stage_change() override;
	void on_set_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
	void on_draw() override;

	void draw_effects(MultiView::Window *win);
	void draw_mesh(MultiView::Window *win, ModelMesh *m, const Array<ModelVertex> &vertex, bool selectable);
	void draw_edges(MultiView::Window *win, ModelMesh *m, const Array<ModelVertex> &vertex, bool selected, bool non_selected, bool as_selected);
	void draw_polygons(MultiView::Window *win, ModelMesh *m, const Array<ModelVertex> &vertex);
	void draw_selection(MultiView::Window *win);
	void draw_creation_preview(MultiView::Window *win);
	void draw_physical(MultiView::Window *win);
	//void draw_all(MultiView::Window *win, Array<ModelVertex> &vertex);

	bool optimize_view() override;

	void easify();

	void create_new_material_for_selection();
	void choose_material_for_selection();

	void add_effects(int type);
	void edit_effects();
	void clear_effects();

	void choose_mouse_function(int f, bool lock);
	void apply_mouse_function(MultiView::MultiView *mv);
	int mouse_action;
	bool lock_action;

	void copy();
	void paste();
	bool copyable();
	bool pasteable();

	ModeModelMeshTexture *mode_model_mesh_texture;
	ModeModelMeshMaterial *mode_model_mesh_material;
	ModeModelMeshDeform *mode_model_mesh_deform;
	ModeModelMeshPaint *mode_model_mesh_paint;

	MeshSelectionMode *selection_mode;
	void set_selection_mode(MeshSelectionMode *mode);
	MeshSelectionMode *selection_mode_vertex;
	MeshSelectionMode *selection_mode_edge;
	MeshSelectionMode *selection_mode_polygon;
	MeshSelectionMode *selection_mode_surface;


	int current_material;
	void set_current_material(int index);

	int current_skin;
	void set_current_skin(int skin);

	void update_vertex_buffers(const Array<ModelVertex> &vertex);
	void fill_selection_buffer(const Array<ModelVertex> &vertex);

	void toggle_select_cw();
	bool select_cw;

	bool allow_draw_hover;
	void set_allow_draw_hover(bool allow);

	nix::VertexBuffer *vb_marked, *vb_hover, *vb_creation, *vb_phys;
};

#endif /* MODEMODELMESH_H_ */
