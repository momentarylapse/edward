/*
 * ModeModelMeshVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHVERTEX_H_
#define MODEMODELMESHVERTEX_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class Mode;
class DataModel;

class ModeModelMeshVertex: public Mode
{
public:
	ModeModelMeshVertex(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshVertex();

	void OnStart();
	void OnEnd();

	void OnUpdate(Observable *o);

	void OnDraw();
	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}
};

extern ModeModelMeshVertex *mode_model_mesh_vertex;

#endif /* MODEMODELMESHVERTEX_H_ */
