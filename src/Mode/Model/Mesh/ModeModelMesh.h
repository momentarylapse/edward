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

class Mode;
class DataModel;

class ModeModelMesh: public Mode
{
public:
	ModeModelMesh(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMesh();

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

	void Draw();
	void DrawWin(int win, irect dest);

	DataModel *data;
};

extern ModeModelMesh *mode_model_mesh;

#endif /* MODEMODELMESH_H_ */
