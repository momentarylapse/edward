/*
 * BrushPanel.cpp
 *
 *  Created on: Jul 6, 2021
 *      Author: michi
 */

#include "BrushPanel.h"
#include "../MultiView/MultiView.h"
//#include "../MultiView/
#include <math.h>

namespace hui {
	void get_style_colors(Panel *p, const string &id, base::map<string,color> &colors);
}


struct WBrushConfig {
	float exponent;
	string name;

	string get_icon(hui::Panel *p) const {
		base::map<string,color> colors;
		hui::get_style_colors(p, "brush-type", colors);

		Image im;
		int n = 48;
		im.create(n, n, White);
		color fg = colors["fg_color"];
		color bg = fg.with_alpha(0);
		for (int i=0; i<n; i++)
			for (int j=0; j<n; j++) {
				vec3 vv = vec3((float)i / (float)n - 0.5f, (float)j / (float)n - 0.5f, 0) * 2;
				float dd = vv * vv;
				float a = exp(-pow(dd, exponent)*2);
				im.set_pixel(i, j, a * fg + (1-a) * bg);

			}
		return hui::set_image(&im, "image:paint-brush-" + name);
	}
};

const int NUM_BRUSHES = 6;
const WBrushConfig BRUSH_PARAM[NUM_BRUSHES] = {
		{400.0f, "extra hard"},
		{4.0f, "hard"},
		{2.0f, "medium"},
		{1.0f, "medium soft"},
		{0.5f, "soft"},
		{0.25f, "extra soft"}
};


BrushPanel::BrushPanel(MultiView::MultiView *mv, const string &res_id) {
	multi_view = mv;

	base_diameter = multi_view->cam.radius * 0.1f;

	from_resource(res_id);

	event("diameter-slider", [=]{ on_diameter_slider(); });
	event("opacity-slider", [=]{ on_opacity_slider(); });
	event("alpha-slider", [=]{ on_alpha_slider(); });

	float _alpha = 1.0f;
	float _opacity = 1.0f;

	for (int i=0; i<NUM_BRUSHES; i++)
		add_string("brush-type", BRUSH_PARAM[i].get_icon(this) + "\\" + BRUSH_PARAM[i].name);
	set_float("diameter-slider", 0.5f);
	set_float("opacity-slider", _opacity);
	set_float("alpha-slider", _alpha);

	set_string("diameter", f2s(base_diameter, 2));
	set_float("opacity", _opacity);
	set_float("alpha", _alpha);
	set_int("brush-type", 2);
	set_color("color", Red);
	check("scale-by-pressure", true);
	check("opacity-by-pressure", true);
}

void BrushPanel::on_diameter_slider() {
	float x = get_float("");
	set_string("diameter", f2s(base_diameter * exp((x - 0.5f) * 4), 2));
}

void BrushPanel::on_opacity_slider() {
	float x = get_float("");
	set_float("opacity", x);
}

void BrushPanel::on_alpha_slider() {
	float x = get_float("");
	set_float("alpha", x);
}


float BrushPanel::radius0() {
	return get_float("diameter") / 2;
}


float BrushPanel::radius() {
	float r = radius0();
	if (is_checked("scale-by-pressure"))
		r *= hui::get_event()->pressure;
	return r;
}

BrushState BrushPanel::prepare(const vec3 &m) {
	BrushState bs;
	bs.m0 = m;

	bs.col = get_color("color");
	bs.col.a = get_float("alpha");

	bs.opacity = get_float("opacity");
	if (is_checked("opacity-by-pressure"))
		bs.opacity *= hui::get_event()->pressure;

	bs.R = radius();

	int type = get_int("brush-type");
	auto bp = BRUSH_PARAM[type];
	bs.exponent = bp.exponent;

	return bs;
}


float BrushState::get(const vec3 &v) const {
	float r = (v - m0).length();
	if (r > R)
		return 0;
	return opacity * exp(-pow(r/R, exponent)*2);
}
