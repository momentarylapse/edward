/*
 * ModeModelSkeleton.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETON_H_
#define MODEMODELSKELETON_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelSkeleton: public Mode<DataModel>
{
public:
	ModeModelSkeleton(ModeBase *_parent);

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;
	void onUpdate(Observable *o, const string &message) override;
	void onUpdateMenu() override;
	void onSetMultiView() override;

	void onDraw() override;
	void onDrawWin(MultiView::Window *win) override;

	void addSubModel();
	void removeSubModel();
	void unlinkSelection();

	void drawSkeleton(MultiView::Window *win, Array<ModelBone> &bone, bool thin = false);


	void chooseMouseFunction(int f);
	int mouse_action;
};

extern ModeModelSkeleton *mode_model_skeleton;

#endif /* MODEMODELSKELETON_H_ */
