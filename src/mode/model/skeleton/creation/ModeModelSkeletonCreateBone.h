/*
 * ModeModelSkeletonCreateBone.h
 *
 *  Created on: 19.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETONCREATEBONE_H_
#define MODEMODELSKELETONCREATEBONE_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelSkeleton;

class ModeModelSkeletonCreateBone: public ModeCreation<ModeModelSkeleton, DataModel> {
public:
	ModeModelSkeletonCreateBone(ModeModelSkeleton *_parent);

	void on_left_button_down() override;

private:
	vec3 pos;
	bool pos_chosen;
	int bone_parent;
};

#endif /* MODEMODELSKELETONCREATEBONE_H_ */
