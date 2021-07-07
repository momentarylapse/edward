/*
 * ModelAnimationTimelinePanel.h
 *
 *  Created on: 23.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_
#define SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_

#include "../../../lib/hui/hui.h"

class TimeLineParasite
{
public:
	virtual ~TimeLineParasite(){}
	virtual void on_timeline_left_button_down(){}
	virtual void on_timeline_left_button_up(){}
	virtual void on_timeline_mouse_move(){}
	virtual void on_timeline_draw(Painter *p){}
};

class ModelAnimationTimelinePanel : public hui::Panel {
public:
	ModelAnimationTimelinePanel();
	virtual ~ModelAnimationTimelinePanel();

	void on_draw(Painter *c);
	void on_mouse_move();
	void on_left_button_down();
	void on_left_button_up();
	void on_mouse_wheel();


	float screen2sample(float x);
	float sample2screen(float x);
	string get_time_str_fuzzy(double t, double dt);
	int get_hover();

	float mx;
	int hover;
	float time_scale, time_offset;

	TimeLineParasite *default_parasite;
	TimeLineParasite *parasite;
	void set_parasite(TimeLineParasite *p);
};

#endif /* SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_ */
