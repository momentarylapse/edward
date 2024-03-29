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
class ModeModel;
class ModelAnimationDialog;
class ModelAnimationTimelinePanel;
class ModeModelAnimationNone;
class ModeModelAnimationSkeleton;
class ModeModelAnimationVertex;

class ModeModelAnimation: public Mode<ModeModel, DataModel> {
public:
	ModeModelAnimation(ModeModel *parent, MultiView::MultiView *mv);
	virtual ~ModeModelAnimation();

	obs::sink in_update;

	class State : public obs::Node<VirtualBase> {
	public:
		static const string MESSAGE_SET_FRAME;
		obs::source out_set_frame{this, "set-frame"};
		~State() {}
	} state;

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update();
	void on_update_menu() override;

	ModelAnimationDialog *dialog;
	ModelAnimationTimelinePanel *timeline;

	ModeModelAnimationNone *mode_model_animation_none;
	ModeModelAnimationSkeleton *mode_model_animation_skeleton;
	ModeModelAnimationVertex *mode_model_animation_vertex;

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

#endif /* MODEMODELANIMATION_H_ */
