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

class ModeModelSkeletonAttachVertices: public ModeCreation<DataModel>
{
public:
	ModeModelSkeletonAttachVertices(ModeBase *_parent, int _bone_index);

	virtual void onStart();
	virtual void onEnd();

	virtual void onKeyDown(int key_code);

	virtual void onDrawWin(MultiView::Window *win);

	void onUpdate(Observable *o, const string &message);

private:
	int bone_index;
};

#endif /* MODEMODELSKELETONATTACHVERTICES_H_ */
