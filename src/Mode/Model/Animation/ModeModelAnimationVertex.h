/*
 * ModeModelAnimationVertex.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONVERTEX_H_
#define MODEMODELANIMATIONVERTEX_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class Mode;
class DataModel;

class ModeModelAnimationVertex: public Mode
{
public:
	ModeModelAnimationVertex(Mode *_parent, DataModel *_data);
	virtual ~ModeModelAnimationVertex();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

	CHuiWindow *dialog;

	void UpdateVertices();
	Array<ModelVertex> vertex;
};

extern ModeModelAnimationVertex *mode_model_animation_vertex;

#endif /* MODEMODELANIMATIONVERTEX_H_ */
