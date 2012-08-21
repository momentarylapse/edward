/*
 * ActionModelSkinVerticesFromProjection.h
 *
 *  Created on: 10.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELSKINVERTICESFROMPROJECTION_H_
#define ACTIONMODELSKINVERTICESFROMPROJECTION_H_

#include "../../../Action.h"
#include "../../../../Data/Model/DataModel.h"
#include "../../../../Data/Model/SkinGenerator.h"

class MultiView;

class ActionModelSkinVerticesFromProjection : public Action
{
public:
	ActionModelSkinVerticesFromProjection(DataModel *m, MultiView *mv);
	virtual ~ActionModelSkinVerticesFromProjection();
	string name(){	return "ModelSkinVerticesFromProjection";	}

	void *execute(Data *d);
	void undo(Data *d);

private:
	Array<int> surface;
	Array<int> triangle;
	Array<int> vert_on_tria;
	Array<vector> old_pos[MODEL_MAX_TEXTURES];
	SkinGenerator sg;
};

#endif /* ACTIONMODELSKINVERTICESFROMPROJECTION_H_ */
