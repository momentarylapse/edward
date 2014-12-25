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
class ModelAnimationTimelinePanel;

class ModeModelAnimation: public Mode<DataModel>, public Observable
{
public:
	ModeModelAnimation(ModeBase *parent);
	virtual ~ModeModelAnimation();

	static const string MESSAGE_SET_FRAME;

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	ModelAnimationDialog *dialog;
	ModelAnimationTimelinePanel *timeline;

	ModelMove *empty_move;

	ModelMove *cur_move();
	int current_move, current_frame;
	Array<ModelVertex> vertex;
	Array<ModelBone> bone;

	float time_scale;
	float time_param;
	bool playing;
	float play_loop;
	float sim_frame_time;

	HuiTimer timer;

	void idleFunction();

	void setCurrentMove(int move);
	int getFirstMove();
	void setCurrentFrame(int frame);
	void setCurrentFrameNext();
	void setCurrentFramePrevious();
	void deleteCurrentFrame();
	void duplicateCurrentFrame();

	void updateAnimation();
	void updateSkeleton();
	void iterateAnimation(float dt);

	void getTimeInterpolation(int &frame0, int &frame1, float &t);
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
