/*
 * ModeModelSkeletonCreateBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETONCREATEBONE_H_
#define MODEMODELSKELETONCREATEBONE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelSkeletonCreateBone: public ModeCreation
{
public:
	ModeModelSkeletonCreateBone(Mode *_parent);
	virtual ~ModeModelSkeletonCreateBone();

	void OnMouseMove();
	void OnLeftButtonDown();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	vector pos;
	bool pos_chosen;
	int bone_parent;
};

#endif /* MODEMODELSKELETONCREATEBONE_H_ */
