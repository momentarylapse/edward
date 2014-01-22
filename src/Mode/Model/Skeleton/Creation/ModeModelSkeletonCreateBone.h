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

class ModeModelSkeletonCreateBone: public ModeCreation<DataModel>
{
public:
	ModeModelSkeletonCreateBone(ModeBase *_parent);
	virtual ~ModeModelSkeletonCreateBone();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();

	virtual void OnDrawWin(MultiView::Window *win);

private:
	vector pos;
	bool pos_chosen;
	int bone_parent;
};

#endif /* MODEMODELSKELETONCREATEBONE_H_ */
