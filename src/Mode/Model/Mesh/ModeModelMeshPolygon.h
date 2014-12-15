/*
 * ModeModelMeshPolygon.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHPOLYGON_H_
#define MODEMODELMESHPOLYGON_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;
class NixVertexBuffer;

class ModeModelMeshPolygon: public Mode<DataModel>
{
public:
	ModeModelMeshPolygon(ModeBase *parent);

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdateMenu();
	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
	void drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void drawSelection(MultiView::Window *win);


	void setMaterialMarked();
	void setMaterialMouseOver();
	void setMaterialCreation();

	void fillSelectionBuffers(Array<ModelVertex> &vertex);

	void toggleSelectCW();
	bool select_cw;

	NixVertexBuffer *vb_model, *vb_model2, *vb_model3, *vb_model4;
	NixVertexBuffer *vb_marked, *vb_hover, *vb_creation;
};

extern ModeModelMeshPolygon *mode_model_mesh_polygon;

#endif /* MODEMODELMESHPOLYGON_H_ */
