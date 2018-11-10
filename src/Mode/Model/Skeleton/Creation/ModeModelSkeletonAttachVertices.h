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

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;

	void on_draw_win(MultiView::Window *win) override;

	void on_set_multi_view() override;

	void on_update(Observable *o, const string &message) override;

private:
	int bone_index;
};

#endif /* MODEMODELSKELETONATTACHVERTICES_H_ */
