/*
 * ModelAnimationTimelinePanel.h
 *
 *  Created on: 23.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_
#define SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"

class TimeLineParasite
{
public:
	virtual ~TimeLineParasite(){}
	virtual void onTimelineLeftButtonDown(){}
	virtual void onTimelineLeftButtonUp(){}
	virtual void onTimelineMouseMove(){}
	virtual void onTimelineDraw(Painter *p){}
};

class ModelAnimationTimelinePanel : public hui::Panel, public Observer
{
public:
	ModelAnimationTimelinePanel();
	virtual ~ModelAnimationTimelinePanel();

	void onDraw(Painter *c);
	void onMouseMove();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMouseWheel();


	float screen2sample(float x);
	float sample2screen(float x);
	string get_time_str_fuzzy(double t, double dt);
	void updateHover();

	virtual void onUpdate(Observable *o, const string &message);

	float mx;
	int hover;
	float time_scale, time_offset;

	TimeLineParasite *default_parasite;
	TimeLineParasite *parasite;
	void setParasite(TimeLineParasite *p);
};

#endif /* SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_ */
