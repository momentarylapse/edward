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
class NixVertexBuffer;
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

	virtual void onDrawWin(MultiView::Window *win);
	virtual void onDraw();

	void drawEffects(MultiView::Window *win);
	void drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected);
	void drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void drawSelection(MultiView::Window *win);

	virtual bool optimizeView();

	void easify();

	void createNewMaterialForSelection();
	void chooseMaterialForSelection();

	void chooseMouseFunction(int f);
	void applyMouseFunction(MultiView::MultiView *mv);

	void addEffects(int type);
	void editEffects();
	void clearEffects();

	ModelMaterialDialog *material_dialog;
	void showMaterialDialog();
	void closeMaterialDialog();
	void toggleMaterialDialog();

	int mouse_action;

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


	void setMaterialMarked();
	void setMaterialMouseOver();
	void setMaterialCreation();

	void fillSelectionBuffer(Array<ModelVertex> &vertex);

	void toggleSelectCW();
	bool select_cw;

	NixVertexBuffer *vb_model[5];
	NixVertexBuffer *vb_marked, *vb_hover, *vb_creation;
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
