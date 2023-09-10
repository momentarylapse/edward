/*
 * ModeModelMeshDeformCylinder.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModeModelMeshDeformCylinder.h"


#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "../../../../data/model/geometry/GeometryCylinder.h"
#include "../../../../EdwardWindow.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/kaba/kaba.h"
#include "../../../../lib/math/random.h"
#include "../../../../multiview/MultiView.h"
#include "../../../../multiview/Window.h"
#include "../../../../multiview/DrawingHelper.h"
#include "../../../../multiview/ColorScheme.h"
#include "../../../../action/ActionGroup.h"
#include "../../../../action/model/mesh/vertex/helper/ActionModelMoveVertex.h"
#include "../selection/MeshSelectionModePolygon.h"
#include "ModeModelMeshDeformCylinder.h"

const int CYLINDER_EDGES = 24;
const int CYLINDER_RINGS = 24;


 ModeModelMeshDeformCylinder::ModeModelMeshDeformCylinder(ModeModelMesh *_parent) :
	ModeCreation<ModeModelMesh, DataModel>("ModelMeshDeformCylinder", _parent)
{
	geo = nullptr;
	has_preview = false;

	param.add(vec3(0,0,1));
	param.add(vec3(0.25f,0.25f,1));
	param.add(vec3(0.5f,0.5f,1));
	param.add(vec3(0.75f,0.75f,1));
	param.add(vec3(1,1,1));

	inter = new Interpolator<float>(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);

	message = _("drag rings, [Control] + click -> copy, [Ctrl + Delete] -> delete, [Ctrl + Return] -> done");

	hover = -1;
	radius = 1;
}

ModeModelMeshDeformCylinder::~ModeModelMeshDeformCylinder() {
	delete inter;
}

vec3 get_ev(mat3 &m) {
	vec3 v = vec3::EZ;
	float vmax = vec3::dot(v, m * v);
	Random r;
	for (int i=0; i<10000; i++) {
		vec3 vv = r.dir();
		float val = vec3::dot(vv, m * vv);
		if (val < vmax) {
			vmax = val;
			v = vv;
		}
	}
	return v;
}

void get_axis(DataModel *data, vec3 axis[2], float &radius) {
	vec3 m = v_0;
	int n = 0;
	foreachi(ModelVertex &v, data->mesh->vertex, i) {
		if (v.is_selected) {
			m += v.pos;
			n ++;
		}
	}
	m /= n;
	mat3 I;
	memset(&I, 0, sizeof(I));
	foreachi(ModelVertex &v, data->mesh->vertex, i) {
		if (v.is_selected) {
			vec3 r = v.pos - m;
			I._00 += r.y*r.y + r.z*r.z;
			I._11 += r.x*r.x + r.z*r.z;
			I._22 += r.x*r.x + r.y*r.y;
			I._01 -= r.x*r.y;
			I._02 -= r.x*r.z;
			I._12 -= r.y*r.z;
		}
	}
	I._10 = I._01;
	I._20 = I._02;
	I._21 = I._12;

	vec3 dir = get_ev(I);
	//axis[0] = axis[1] = m;
	float ll[2] = {0,0};
	radius = 0;
	foreachi(ModelVertex &v, data->mesh->vertex, i)
		if (v.is_selected) {
			float l = vec3::dot(v.pos - m, dir);
			ll[0] = min(ll[0], l);
			ll[1] = max(ll[1], l);
			float r = VecLineDistance(v.pos, m, m + dir);
			radius = max(radius, r);
		}
	axis[0] = m + ll[0] * dir;
	axis[1] = m + ll[1] * dir;
}

void ModeModelMeshDeformCylinder::on_start() {
	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);

	foreachi(ModelVertex &v, data->mesh->vertex, i)
		if (v.is_selected) {
			old_pos.add(v.pos);
			index.add(i);
		}

	get_axis(data, axis, radius);
	dir = axis[1] - axis[0];
	dir.normalize();

	update_params();
	hover = -1;
}

void ModeModelMeshDeformCylinder::on_end() {
	if (has_preview)
		restore();
}

Array<vec3> sort_vectors_by_x(Array<vec3> &p) {
	Array<vec3> pp = p;
	for (int i=0; i<pp.num; i++)
		for (int j=i+1; j<pp.num; j++)
			if (pp[i].x > pp[j].x)
				pp.swap(i, j);
	return pp;
}

void ModeModelMeshDeformCylinder::update_params() {
	inter->clear();
	float last = 0;
	auto p = sort_vectors_by_x(param);
	for (vec3 &pp: p) {
		inter->add(pp.z, pp.y - last);
		last = pp.y;
	}

	preview();
}


void ModeModelMeshDeformCylinder::on_draw_post() {
	if (hover >= 0) {
		nix::set_shader(nix::Shader::default_2d.get());
		set_color(scheme.TEXT);
		draw_str(multi_view->m.x + 40, multi_view->m.y + 40, format("radius: %s  (%.1f%%)", multi_view->format_length(param[hover].z * radius), param[hover].z * 100));
	}
}

void ModeModelMeshDeformCylinder::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);

	if (geo) {
		set_material_creation(0.3f);
		geo->build(nix::vb_temp);
		nix::draw_triangles(nix::vb_temp);
	}

	set_line_width(scheme.LINE_WIDTH_MEDIUM);
	set_color(scheme.CREATION_LINE);
	nix::disable_alpha();
	nix::set_z(false, false);
	draw_line(axis[0], axis[1]);

	vec3 e1 = dir.ortho();
	vec3 e2 = vec3::cross(dir, e1);
	foreachi(vec3 &p, param, ip) {
		set_color((ip == hover) ? scheme.SELECTION : scheme.CREATION_LINE);
		vec3 m = axis[0] + (axis[1] - axis[0]) * p.y;
		draw_circle(m, dir, radius * p.z);
	}

	nix::set_z(true, true);
}

inline bool hover_line(const vec3 &a, const vec3 &b, const vec3 &m, vec3 &tp) {
	const float r = 8;
	if ((b-a).length_sqr() < r*r)
		if (((a+b)/2 - m).length_sqr() < r*r) {
			tp = (a+b)/2;
			return true;
		}

	vec3 p = VecLineNearestPoint(m, a, b);
	if ((p - m).length_sqr() < r*r) {
		if (p.between(a, b)) {
			tp = p;
			return true;
		}
	}
	return false;
}

void ModeModelMeshDeformCylinder::update_hover() {
	hover = -1;
	vec3 e1 = dir.ortho();
	vec3 e2 = vec3::cross(dir, e1);
	foreachi(vec3 &p, param, ip) {
		vec3 m = axis[0] + (axis[1] - axis[0]) * p.y;
		vec3 v =  multi_view->mouse_win->project(m + e1 * radius * p.z);
		v.z = 0;
		for (int i=1; i<=CYLINDER_EDGES; i++) {
			float ang = (float)i / (float)CYLINDER_EDGES * 2 * pi;
			vec3 w = multi_view->mouse_win->project(m + (e1 * cos(ang) + e2 * sin(ang)) * radius * p.z);
			w.z = 0;
			if (hover_line(v, w, {multi_view->m,0}, hover_tp)) {
				hover = ip;
				orig_param = p;
				return;
			}
			v = w;
		}
	}
}

vec3 ModeModelMeshDeformCylinder::transform(const vec3 &v) {
	//vector e1 = dir.ortho();
	//vector e2 = dir ^ e1;

	float ll = (axis[1] - axis[0]).length();

	// axial and radial cylinder components
	float l = vec3::dot(v - axis[0], dir);
	vec3 r = v - axis[0] - l * dir;

	float f = inter->get(l / ll);

	r *= f;

	return axis[0] + l * dir + r;
}

void ModeModelMeshDeformCylinder::preview() {
	if (has_preview)
		restore();

	geo = new GeometryCylinder(axis[0], axis[1], radius, CYLINDER_RINGS, CYLINDER_EDGES, GeometryCylinder::END_OPEN);

	for (auto &v: geo->vertex)
		v.pos = transform(v.pos);

	for (int vi: index)
		data->mesh->vertex[vi].pos = transform(data->mesh->vertex[vi].pos);

	data->out_changed();
	//mode_model_mesh->updateVertexBuffers(data->vertex);

	has_preview = true;
	multi_view->force_redraw();
}

void ModeModelMeshDeformCylinder::on_mouse_move() {
	if (hui::get_event()->lbut) {
		if (hover >= 0) {
			vec3 m = {multi_view->m, 0};
			vec3 a0 = multi_view->mouse_win->project(axis[0]);
			vec3 a1 = multi_view->mouse_win->project(axis[1]);
			vec3 pp = hover_tp;//multi_view->mouse_win->project(hover_tp);
			m.z = a0.z = a1.z = pp.z = 0;

			float dpp = VecLineDistance(pp, a0, a1);
			float dm = VecLineDistance(m, a0, a1);
			if (dpp > 10) {
				param[hover].z = orig_param.z * dm / dpp;
			}

			if ((hover != 0) and (hover != param.num-1)) {
				vec3 d = (a1 - a0);
				float la = d.length();
				d.normalize();
				float lm = vec3::dot(m - pp, d);
				if (la > 30) {
					param[hover].y = clamp(orig_param.y +  lm / la, 0.0f, 1.0f);
					param[hover].x = param[hover].y;
				}
			}
			update_params();
		}
	} else {
		update_hover();
	}
}

void ModeModelMeshDeformCylinder::on_left_button_down() {
	if (hover >= 0) {
		if (ed->get_key(hui::KEY_CONTROL)) {
			int n = hover;
			if (n == 0)
				n = 1;
			param.insert(param[hover], n);
			hover = n;
			update_params();
		}
	}

}

void ModeModelMeshDeformCylinder::on_key_down(int k) {
	if (k == hui::KEY_DELETE + hui::KEY_CONTROL) {
		if ((hover != 0) and (hover != param.num -1)) {
			param.erase(hover);
			hover = -1;
			update_params();
		}
	}
}

void ModeModelMeshDeformCylinder::restore() {
	if (geo) {
		delete geo;
		geo = nullptr;
	}

	foreachi (int vi, index, i)
		data->mesh->vertex[vi].pos = old_pos[i];
	has_preview = false;

}

void ModeModelMeshDeformCylinder::on_ok() {
	if (has_preview)
		restore();

	data->begin_action_group("deformation");
	for (int vi: index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->mesh->vertex[vi].pos)));

	data->end_action_group();

	abort();
}

void ModeModelMeshDeformCylinder::on_command(const string& id) {
	if (id == "finish-action")
		on_ok();
}
