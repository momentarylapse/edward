/*
 * ModeModelMeshDeformCylinder.cpp
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#include "ModeModelMeshDeformCylinder.h"


#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/kaba/kaba.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../Action/ActionGroup.h"
#include "../../../../Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.h"
#include "../Selection/MeshSelectionModePolygon.h"
#include "ModeModelMeshDeformCylinder.h"

const int CYLINDER_EDGES = 24;
const int CYLINDER_RINGS = 24;


 ModeModelMeshDeformCylinder::ModeModelMeshDeformCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshDeformCylinder", _parent)
{
	geo = NULL;
	has_preview = false;

	param.add(vector(0,0,1));
	param.add(vector(0.25f,0.25f,1));
	param.add(vector(0.5f,0.5f,1));
	param.add(vector(0.75f,0.75f,1));
	param.add(vector(1,1,1));

	inter = new Interpolator<float>(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);

	message = _("drag rings, [Control] + click -> copy, [Delete] -> delete, [Ctrl + Return] -> done");

	hover = -1;
	radius = 1;
}

ModeModelMeshDeformCylinder::~ModeModelMeshDeformCylinder()
{
	delete inter;
}

vector get_ev(matrix3 &m)
{
	vector v = vector::EZ;
	float vmax = v * (m * v);
	Random r;
	for (int i=0; i<10000; i++){
		vector vv = r.dir();
		float val = vv * (m * vv);
		if (val < vmax){
			vmax = val;
			v = vv;
		}
	}
	return v;
}

void get_axis(DataModel *data, vector axis[2], float &radius)
{
	vector m = v_0;
	int n = 0;
	foreachi(ModelVertex &v, data->vertex, i){
		if (v.is_selected){
			m += v.pos;
			n ++;
		}
	}
	m /= n;
	matrix3 I;
	memset(&I, 0, sizeof(I));
	foreachi(ModelVertex &v, data->vertex, i){
		if (v.is_selected){
			vector r = v.pos - m;
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

	vector dir = get_ev(I);
	//axis[0] = axis[1] = m;
	float ll[2] = {0,0};
	radius = 0;
	foreachi(ModelVertex &v, data->vertex, i)
		if (v.is_selected){
			float l = (v.pos - m) * dir;
			ll[0] = min(ll[0], l);
			ll[1] = max(ll[1], l);
			float r = VecLineDistance(v.pos, m, m + dir);
			radius = max(radius, r);
		}
	axis[0] = m + ll[0] * dir;
	axis[1] = m + ll[1] * dir;
}

void ModeModelMeshDeformCylinder::on_start()
{
	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);

	foreachi(ModelVertex &v, data->vertex, i)
		if (v.is_selected){
			old_pos.add(v.pos);
			index.add(i);
		}

	get_axis(data, axis, radius);
	dir = axis[1] - axis[0];
	dir.normalize();

	updateParams();
	hover = -1;
}

void ModeModelMeshDeformCylinder::on_end()
{
	if (has_preview)
		restore();
}

Array<vector> sort_vectors_by_x(Array<vector> &p)
{
	Array<vector> pp = p;
	for (int i=0; i<pp.num; i++)
		for (int j=i+1; j<pp.num; j++)
			if (pp[i].x > pp[j].x)
				pp.swap(i, j);
	return pp;
}

void ModeModelMeshDeformCylinder::updateParams()
{
	inter->clear();
	float last = 0;
	auto p = sort_vectors_by_x(param);
	for (vector &pp: p){
		inter->add(pp.z, pp.y - last);
		last = pp.y;
	}

	preview();
}

void ModeModelMeshDeformCylinder::on_draw_win(MultiView::Window* win)
{
	parent->on_draw_win(win);

	if (geo){
		ModeModel::set_material_creation(0.3f);
		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);
	}

	MultiView::set_wide_lines(3);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(false, false);
	nix::SetColor(multi_view->ColorCreationLine);
	nix::DrawLine3D(axis[0], axis[1]);

	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	foreachi(vector &p, param, ip){
		nix::SetColor((ip == hover) ? Red : multi_view->ColorCreationLine);
		vector m = axis[0] + (axis[1] - axis[0]) * p.y;
		vector v = m + e1 * radius * p.z;
		for (int i=1; i<=CYLINDER_EDGES; i++){
			float ang = (float)i / (float)CYLINDER_EDGES * 2 * pi;
			vector w = m + (e1 * cos(ang) + e2 * sin(ang)) * radius * p.z;
			nix::DrawLine3D(w, v);
			v = w;
		}
	}

	nix::SetZ(true, true);
}

inline bool hover_line(vector &a, vector &b, vector &m, vector &tp)
{
	const float r = 8;
	if ((b-a).length_sqr() < r*r)
		if (((a+b)/2 - m).length_sqr() < r*r){
			tp = (a+b)/2;
			return true;
		}

	vector p = VecLineNearestPoint(m, a, b);
	if ((p - m).length_sqr() < r*r){
		if (p.between(a, b)){
			tp = p;
			return true;
		}
	}
	return false;
}

void ModeModelMeshDeformCylinder::updateHover()
{
	hover = -1;
	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	foreachi(vector &p, param, ip){
		vector m = axis[0] + (axis[1] - axis[0]) * p.y;
		vector v =  multi_view->mouse_win->project(m + e1 * radius * p.z);
		v.z = 0;
		for (int i=1; i<=CYLINDER_EDGES; i++){
			float ang = (float)i / (float)CYLINDER_EDGES * 2 * pi;
			vector w = multi_view->mouse_win->project(m + (e1 * cos(ang) + e2 * sin(ang)) * radius * p.z);
			w.z = 0;
			if (hover_line(v, w, multi_view->m, hover_tp)){
				hover = ip;
				orig_param = p;
				return;
			}
			v = w;
		}
	}
}

vector ModeModelMeshDeformCylinder::transform(const vector &v)
{
	//vector e1 = dir.ortho();
	//vector e2 = dir ^ e1;

	float ll = (axis[1] - axis[0]).length();

	// axial and radial cylinder components
	float l = (v - axis[0]) * dir;
	vector r = v - axis[0] - l * dir;

	float f = inter->get(l / ll);

	r *= f;

	return axis[0] + l * dir + r;
}

void ModeModelMeshDeformCylinder::onPreview()
{}

void ModeModelMeshDeformCylinder::preview()
{
	if (has_preview)
		restore();

	geo = new GeometryCylinder(axis[0], axis[1], radius, CYLINDER_RINGS, CYLINDER_EDGES, GeometryCylinder::END_OPEN);

	for (auto &v: geo->vertex)
		v.pos = transform(v.pos);

	for (int vi: index)
		data->vertex[vi].pos = transform(data->vertex[vi].pos);

	data->notify();
	//mode_model_mesh->updateVertexBuffers(data->vertex);

	has_preview = true;
	ed->force_redraw();
}

void ModeModelMeshDeformCylinder::on_mouse_move()
{
	if (hui::GetEvent()->lbut){
		if (hover >= 0){
			vector m = multi_view->m;
			vector a0 = multi_view->mouse_win->project(axis[0]);
			vector a1 = multi_view->mouse_win->project(axis[1]);
			vector pp = hover_tp;//multi_view->mouse_win->project(hover_tp);
			m.z = a0.z = a1.z = pp.z = 0;

			float dpp = VecLineDistance(pp, a0, a1);
			float dm = VecLineDistance(m, a0, a1);
			if (dpp > 10){
				param[hover].z = orig_param.z * dm / dpp;
			}

			if ((hover != 0) and (hover != param.num-1)){
				vector d = (a1 - a0);
				float la = d.length();
				d.normalize();
				float lm = (m - pp) * d;
				if (la > 30){
					param[hover].y = clampf(orig_param.y +  lm / la, 0, 1);
					param[hover].x = param[hover].y;
				}
			}
			updateParams();
		}
	}else{
		updateHover();
	}
}

void ModeModelMeshDeformCylinder::on_left_button_down()
{
	if (hover >= 0){
		if (ed->get_key(hui::KEY_CONTROL)){
			int n = hover;
			if (n == 0)
				n = 1;
			param.insert(param[hover], n);
			hover = n;
			updateParams();
		}
	}

}

void ModeModelMeshDeformCylinder::on_left_button_up()
{
}

void ModeModelMeshDeformCylinder::on_key_down(int k)
{
	if (k == hui::KEY_DELETE){
		if ((hover != 0) and (hover != param.num -1)){
			param.erase(hover);
			hover = -1;
			updateParams();
		}
	}
}

void ModeModelMeshDeformCylinder::restore()
{
	if (geo){
		delete geo;
		geo = NULL;
	}

	foreachi (int vi, index, i)
		data->vertex[vi].pos = old_pos[i];
	has_preview = false;

}

void ModeModelMeshDeformCylinder::onOk()
{
	if (has_preview)
		restore();

	data->begin_action_group("deformation");
	for (int vi: index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->vertex[vi].pos)));

	data->end_action_group();

	abort();
}

void ModeModelMeshDeformCylinder::onClose()
{
	abort();
}

void ModeModelMeshDeformCylinder::on_command(const string& id)
{
	if (id == "finish-action")
		onOk();
}
