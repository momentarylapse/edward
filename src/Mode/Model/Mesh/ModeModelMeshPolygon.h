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
	virtual ~ModeModelMeshPolygon();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdateMenu();
	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
	void DrawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex);
	void DrawSelection(MultiView::Window *win);


	void SetMaterialMarked();
	void SetMaterialMouseOver();
	void SetMaterialCreation();

	void FillSelectionBuffers(Array<ModelVertex> &vertex);

	void ToggleSelectCW();
	bool SelectCW;

	NixVertexBuffer *VBModel, *VBModel2, *VBModel3, *VBModel4;
	NixVertexBuffer *VBMarked, *VBMouseOver, *VBCreation;
};

extern ModeModelMeshPolygon *mode_model_mesh_polygon;

#endif /* MODEMODELMESHPOLYGON_H_ */
