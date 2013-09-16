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

class ModeModelAnimation: public Mode<DataModel>, public Observable, public HuiEventHandler
{
public:
	ModeModelAnimation(ModeBase *parent);
	virtual ~ModeModelAnimation();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	ModelAnimationDialog *dialog;

	ModelMove *EmptyMove;

	ModelMove *move;
	int CurrentMove, CurrentFrame;
	Array<ModelVertex> vertex;

	float TimeScale;
	float TimeParam;
	bool Playing;
	float PlayLoop;
	float SimFrame;

	HuiTimer timer;

	void IdleFunction();

	void SetCurrentMove(int move);
	void SetCurrentFrame(int frame);
	void SetCurrentFrameNext();
	void SetCurrentFramePrevious();
	void AnimationDeleteCurrentFrame();
	void AnimationDuplicateCurrentFrame();

	void UpdateAnimation();
	void UpdateSkeleton();
	void IterateAnimation(float dt);
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
