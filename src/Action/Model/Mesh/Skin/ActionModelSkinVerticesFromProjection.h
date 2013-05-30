/*
 * ActionModelSkinVerticesFromProjection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSKINVERTICESFROMPROJECTION_H_
#define ACTIONMODELSKINVERTICESFROMPROJECTION_H_

#include "../../../Action.h"
#include "../../../../x/material.h"
#include "../../../../Data/Model/SkinGenerator.h"
class DataModel;

class MultiView;

class ActionModelSkinVerticesFromProjection : public Action
{
public:
	ActionModelSkinVerticesFromProjection(DataModel *m, MultiView *mv);
	virtual ~ActionModelSkinVerticesFromProjection();
	virtual string name(){	return "ModelSkinVerticesFromProjection";	}
	virtual string message(){	return "SkinChange";	}

	virtual void *execute(Data *d);
	virtual void undo(Data *d);

private:
	Array<int> surface;
	Array<int> tria;
	Array<int> vert_on_tria;
	Array<vector> old_pos[MATERIAL_MAX_TEXTURES];
	SkinGenerator sg;
};

#endif /* ACTIONMODELSKINVERTICESFROMPROJECTION_H_ */
