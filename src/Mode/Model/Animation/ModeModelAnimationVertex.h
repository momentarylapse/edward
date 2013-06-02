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

class DataModel;

class ModeModelAnimationVertex: public Mode<DataModel>
{
public:
	ModeModelAnimationVertex(ModeBase *parent);
	virtual ~ModeModelAnimationVertex();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	CHuiWindow *dialog;

	void UpdateVertices();
};

extern ModeModelAnimationVertex *mode_model_animation_vertex;

#endif /* MODEMODELANIMATIONVERTEX_H_ */
