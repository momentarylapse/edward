/*
 * BrushPanel.h
 *
 *  Created on: Jul 6, 2021
 *      Author: michi
 */

#pragma once

#include "../lib/hui/hui.h"
#include "../lib/math/vector.h"

namespace MultiView {
	class MultiView;
}

class BrushState {
public:
	float opacity = 1.0f;
	float exponent = 2.0f;
	float R = 1.0f;
	vector m0 = vector::ZERO;
	color col = White;

	float get(const vector &v) const;
};

class BrushPanel : public hui::Panel {
public:
	BrushPanel(MultiView::MultiView *mv, const string &res_id);

	void on_diameter_slider();
	void on_opacity_slider();
	void on_alpha_slider();

	BrushState prepare(const vector &m);
	float radius();
	float radius0();


private:
	MultiView::MultiView *multi_view;
	float base_diameter;
};
