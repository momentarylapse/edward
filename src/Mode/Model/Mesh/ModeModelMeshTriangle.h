/*
 * ModeModelMeshSkin.h
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHSKIN_H_
#define MODEMODELMESHSKIN_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class Mode;
class DataModel;

class ModeModelMeshTriangle: public Mode
{
public:
	ModeModelMeshTriangle(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshTriangle();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdateMenu();
	void OnUpdate(Observable *o);

	void OnDraw();
	void OnDrawWin(int win, irect dest);
	void DrawTrias();

	void FillSelectionBuffers();

	DataModel *data;
	Data *GetData(){	return data;	}

	void ToggleSelectCW();
	bool SelectCW;

	int VBModel, VBModel2, VBModel3, VBModel4;
	int VBMarked, VBMouseOver, VBCreation;
};

extern ModeModelMeshTriangle *mode_model_mesh_triangle;

#endif /* MODEMODELMESHSKIN_H_ */
