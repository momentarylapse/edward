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

class Mode;
class DataModel;

class ModeModelMeshPolygon: public Mode
{
public:
	ModeModelMeshPolygon(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshPolygon();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdateMenu();
	void OnUpdate(Observable *o);

	void OnDraw();
	void OnDrawWin(int win);
	void DrawPolygons();

	void FillSelectionBuffers();

	DataModel *data;
	Data *GetData(){	return data;	}

	void ToggleSelectCW();
	bool SelectCW;

	int VBModel, VBModel2, VBModel3, VBModel4;
	int VBMarked, VBMouseOver, VBCreation;
};

extern ModeModelMeshPolygon *mode_model_mesh_polygon;

#endif /* MODEMODELMESHPOLYGON_H_ */
