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

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;

	void onDrawWin(MultiView::Window *win) override;

	void onSetMultiView() override;

	void onUpdate(Observable *o, const string &message) override;

private:
	int bone_index;
};

#endif /* MODEMODELSKELETONATTACHVERTICES_H_ */
