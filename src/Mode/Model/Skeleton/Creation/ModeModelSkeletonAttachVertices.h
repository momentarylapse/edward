/*
 * ModeModelSkeletonAttachVertices.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETONATTACHVERTICES_H_
#define MODEMODELSKELETONATTACHVERTICES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelSkeletonAttachVertices: public ModeCreation
{
public:
	ModeModelSkeletonAttachVertices(Mode *_parent, int _bone_index);
	virtual ~ModeModelSkeletonAttachVertices();

	void OnStart();
	void OnEnd();

	void OnKeyDown();

	void OnDrawWin(MultiViewWindow *win);

	DataModel *data;
	Data *GetData(){	return data;	}

private:
	int bone_index;
};

#endif /* MODEMODELSKELETONATTACHVERTICES_H_ */
