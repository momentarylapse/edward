/*
 * ModeModelMeshEdge.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHEDGE_H_
#define MODEMODELMESHEDGE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class Mode;
class DataModel;

class ModeModelMeshEdge: public Mode
{
public:
	ModeModelMeshEdge(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshEdge();

	void OnStart();
	void OnEnd();

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

	void OnDraw();
	void OnDrawWin(int win);

	DataModel *data;
	Data *GetData(){	return data;	}
};

extern ModeModelMeshEdge *mode_model_mesh_edge;

#endif /* MODEMODELMESHEDGE_H_ */
