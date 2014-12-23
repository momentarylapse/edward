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

class ModelAnimationTimelinePanel : public HuiPanel, public Observer
{
public:
	ModelAnimationTimelinePanel();
	virtual ~ModelAnimationTimelinePanel();

	void onDraw();
	void onMouseMove();
	void onLeftButtonDown();
	void onLeftButtonUp();
	void onMouseWheel();


	float screen2sample(float x);
	float sample2screen(float x);
	string get_time_str_fuzzy(double t, double dt);
	void updateHover(float mx);

	virtual void onUpdate(Observable *o, const string &message);

	int hover;
	float time_scale, time_offset;
};

#endif /* SRC_MODE_MODEL_DIALOG_MODELANIMATIONTIMELINEPANEL_H_ */
