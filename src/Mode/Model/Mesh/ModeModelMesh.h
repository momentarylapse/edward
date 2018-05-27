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
class ModelMaterialDialog;
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

	virtual void onStart();
	virtual void onEnter();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();
	virtual void onSelectionChange();
	virtual void onViewStageChange();
	virtual void onSetMultiView();

	virtual void onDrawWin(MultiView::Window *win);
	virtual void onDraw();

	void drawEffects(MultiView::Window *win);
	void drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
	void drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void drawSelection(MultiView::Window *win);
	void drawPhysical(MultiView::Window *win);
	void drawAll(MultiView::Window *win, Array<ModelVertex> &vertex);

	virtual bool optimizeView();

	void easify();

	void createNewMaterialForSelection();
	void chooseMaterialForSelection();

	void addEffects(int type);
	void editEffects();
	void clearEffects();

	ModelMaterialDialog *material_dialog;
	void showMaterialDialog();
	void closeMaterialDialog();
	void toggleMaterialDialog();

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
