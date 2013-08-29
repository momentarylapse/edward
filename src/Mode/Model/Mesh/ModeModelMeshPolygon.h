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

class ModeModelMeshPolygon: public Mode<DataModel>
{
public:
	ModeModelMeshPolygon(ModeBase *parent);
	virtual ~ModeModelMeshPolygon();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdateMenu();
	virtual void OnUpdate(Observable *o);

	virtual void OnDraw();
	virtual void OnDrawWin(MultiViewWindow *win);
	void DrawPolygons(MultiViewWindow *win, Array<ModelVertex> &vertex);
	void DrawSelection(MultiViewWindow *win);


	void SetMaterialMarked();
	void SetMaterialMouseOver();
	void SetMaterialCreation();

	void FillSelectionBuffers(Array<ModelVertex> &vertex);

	void ToggleSelectCW();
	bool SelectCW;

	int VBModel, VBModel2, VBModel3, VBModel4;
	int VBMarked, VBMouseOver, VBCreation;
};

extern ModeModelMeshPolygon *mode_model_mesh_polygon;

#endif /* MODEMODELMESHPOLYGON_H_ */
