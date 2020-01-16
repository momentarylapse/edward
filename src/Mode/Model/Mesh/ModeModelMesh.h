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
namespace nix{
	class VertexBuffer;
};
class MeshSelectionMode;

enum
{
	SELECTION_MODE_VERTEX,
	SELECTION_MODE_EDGE,
	SELECTION_MODE_POLYGON,
	SELECTION_MODE_SURFACE
};

class ModeModelMesh: public Mode<DataModel>, public Observable
{
public:
	ModeModelMesh(ModeBase *parent);
	virtual ~ModeModelMesh();

	static const string MESSAGE_CURRENT_MATERIAL_CHANGE;

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

	void drawEffects(MultiView::Window *win);
	void drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
	void drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void drawSelection(MultiView::Window *win);
	void drawPhysical(MultiView::Window *win);
	void drawAll(MultiView::Window *win, Array<ModelVertex> &vertex);

	bool optimize_view() override;

	void easify();

	void createNewMaterialForSelection();
	void chooseMaterialForSelection();

	void addEffects(int type);
	void editEffects();
	void clearEffects();

	void chooseMouseFunction(int f, bool lock);
	void applyMouseFunction(MultiView::MultiView *mv);
	int mouse_action;
	bool lock_action;

	void copy();
	void paste();
	bool copyable();
	bool pasteable();
	Geometry temp_geo;

	MeshSelectionMode *selection_mode;
	void setSelectionMode(MeshSelectionMode *mode);
	MeshSelectionMode *selection_mode_vertex;
	MeshSelectionMode *selection_mode_edge;
	MeshSelectionMode *selection_mode_polygon;
	MeshSelectionMode *selection_mode_surface;


	int current_material;
	void setCurrentMaterial(int index);

	void updateVertexBuffers(Array<ModelVertex> &vertex);
	void fillSelectionBuffer(Array<ModelVertex> &vertex);

	void toggleSelectCW();
	bool select_cw;

	nix::VertexBuffer *vb_marked, *vb_hover, *vb_creation;
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
