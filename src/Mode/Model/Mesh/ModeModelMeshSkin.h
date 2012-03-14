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

class ModeModelMeshSkin: public Mode
{
public:
	ModeModelMeshSkin(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshSkin();

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);
	void DrawTrias();

	void FillSelectionBuffers();

	DataModel *data;

	int VBModel, VBModel2, VBModel3, VBModel4;
	int VBMarked, VBMouseOver, VBCreation;
};

extern ModeModelMeshSkin *mode_model_mesh_skin;

#endif /* MODEMODELMESHSKIN_H_ */
