/*
 * ModeModelAnimation.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATION_H_
#define MODEMODELANIMATION_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;
class ModelAnimationDialog;

class ModeModelAnimation: public Mode<DataModel>, public Observable
{
public:
	ModeModelAnimation(ModeBase *parent);
	virtual ~ModeModelAnimation();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);

	ModelAnimationDialog *dialog;

	ModelMove *empty_move;

	ModelMove *move;
	int current_move, current_frame;
	Array<ModelVertex> vertex;
	Array<ModelBone> bone;

	float time_scale;
	float time_param;
	bool playing;
	float play_loop;
	float sim_frame;

	HuiTimer timer;

	void idleFunction();

	void setCurrentMove(int move);
	void setCurrentFrame(int frame);
	void setCurrentFrameNext();
	void setCurrentFramePrevious();
	void animationDeleteCurrentFrame();
	void animationDuplicateCurrentFrame();

	void updateAnimation();
	void updateSkeleton();
	void iterateAnimation(float dt);
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
