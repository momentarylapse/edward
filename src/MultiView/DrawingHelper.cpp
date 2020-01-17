/*
 * DrawingHelper.cpp
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#include "../lib/nix/nix.h"
#include "MultiView.h"
#include "Window.h"


namespace MultiView{
	extern nix::Shader *shader_selection;
	extern nix::Shader *shader_lines_3d;
	extern nix::Shader *shader_lines_3d_colored;
	extern nix::Shader *shader_lines_3d_colored_wide;
}

void set_wide_lines(float width) {
	if (width == 1.0f) {
		nix::SetShader(MultiView::shader_lines_3d_colored);
	} else {
		auto s = MultiView::shader_lines_3d_colored_wide;
		nix::SetShader(s);
		int loc_tw = s->get_location("target_width");
		int loc_th = s->get_location("target_height");
		int loc_lw = s->get_location("line_width");
		s->set_float(loc_tw, nix::target_width);
		s->set_float(loc_th, nix::target_height);
		s->set_float(loc_lw, width);
	}
}


void draw_circle(const vector &pos, const vector &n, float radius) {

	vector e1 = n.ortho();
	vector e2 = n ^ e1;
	e1 *= radius;
	e2 *= radius;
	int N = 64;
	for (int i=0;i<N;i++) {
		float w1 = i * 2 * pi / N;
		float w2 = (i + 1) * 2 * pi / N;
		nix::DrawLine3D(pos + sin(w1) * e1 + cos(w1) * e2, pos + sin(w2) * e1 + cos(w2) * e2);
	}
}


void draw_helper_line(MultiView::Window *win, const vector &a, const vector &b) {
	nix::SetZ(false, false);
	nix::SetColor(MultiView::MultiView::ColorText);
	set_wide_lines(3.0f);
	nix::DrawLine3D(a, b);
	set_wide_lines(1.0f);
	//nix::SetZ(true, true);
	vector pa = win->project(a);
	vector pb = win->project(b);
	//vector d = (pb - pa).normalized();
	//vector e = d ^ vector::EZ;
	float r = 3;
	nix::SetShader(nix::default_shader_2d);
	nix::DrawRect(pa.x-r, pa.x+r, pa.y-r, pa.y+r, 0);
	nix::DrawRect(pb.x-r, pb.x+r, pb.y-r, pb.y+r, 0);
}
