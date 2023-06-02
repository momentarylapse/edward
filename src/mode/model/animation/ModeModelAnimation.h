/*
 * ModeModelAnimation.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATION_H_
#define MODEMODELANIMATION_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"
#include "../../../lib/os/time.h"

class DataModel;
class ModelAnimationDialog;
class ModelAnimationTimelinePanel;

class ModeModelAnimation: public Mode<DataModel> {
public:
	ModeModelAnimation(ModeBase *parent, MultiView::MultiView *mv);
	virtual ~ModeModelAnimation();

	class State : public Observable<VirtualBase> {
	public:
		static const string MESSAGE_SET_FRAME;
		~State() {}
	} state;

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update();
	void on_update_menu() override;

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

	os::Timer timer;
	int runner;

	void idle_function();

	void set_current_move(int move);
	int getFirstMove();
	void set_current_frame(int frame);
	void set_current_frame_next();
	void set_current_frame_previous();
	void delete_current_frame();
	void duplicate_current_frame();

	void update_animation();
	void update_skeleton();
	void iterate_animation(float dt);

	ModelFrame get_interpolation();
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
