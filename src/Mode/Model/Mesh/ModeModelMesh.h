/*
 * ModeModelMesh.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESH_H_
#define MODEMODELMESH_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/Geometry/Geometry.h"

class DataModel;
namespace nix {
	class VertexBuffer;
};
class MeshSelectionMode;

class ModeModelMesh: public Mode<DataModel>, public Observable {
public:
	ModeModelMesh(ModeBase *parent);
	virtual ~ModeModelMesh();

	static const string MESSAGE_CURRENT_MATERIAL_CHANGE;
	static const string MESSAGE_CURRENT_SKIN_CHANGE;

	void on_start() override;
	void on_enter() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update(Observable *o, const string &message) override;
	void on_update_menu() override;
	void on_selection_change() override;
	void on_view_stage_change() override;
	void on_set_multi_view() override;

	void on_draw_win(MultiView::Window *win) override;
	void on_draw() override;

	void draw_effects(MultiView::Window *win);
	void draw_edges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
	void draw_polygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void draw_selection(MultiView::Window *win);
	void draw_physical(MultiView::Window *win);
	void draw_all(MultiView::Window *win, Array<ModelVertex> &vertex);

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
	Geometry temp_geo;

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

	void update_vertex_buffers(Array<ModelVertex> &vertex);
	void fill_selection_buffer(Array<ModelVertex> &vertex);

	void toggle_select_cw();
	bool select_cw;

	bool allow_draw_hover;
	void set_allow_draw_hover(bool allow);

	nix::VertexBuffer *vb_marked, *vb_hover, *vb_creation, *vb_phys;
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
