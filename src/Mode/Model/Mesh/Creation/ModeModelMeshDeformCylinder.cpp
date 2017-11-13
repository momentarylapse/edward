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

class DeformationCylinderDialog : public hui::Dialog
{
public:
	DeformationCylinderDialog(ModeModelMeshDeformCylinder *_mode) :
		hui::Dialog("", 0, 0, ed, true)
	{
		fromResource("deformation_cylinder_dialog");
		mode = _mode;
		w = h = 0;
		hf = 0.3f;
		hover = -1;
		update();
	}
	void update()
	{
		mode->updateParams();
		redraw("area");
		ed->forceRedraw();
	}

	float w, h, hf;
	int hover;
	complex project(const vector &v)
	{
		return complex(w * v.y, h - h * v.z * hf);

	}
	vector unproject(const complex &c)
	{
		return vector(c.x / w, c.x / w, (h - c.y) / h / hf);

	}

	virtual void onDraw(Painter *p)
	{
		w = p->width;
		h = p->height;
		p->setColor(White);
		p->drawRect(0, 0, w, h);

		p->setColor(color(1, 0.9f, 0.9f, 0.9f));
		float y = project(vector(0,0,1)).y;
		p->drawLine(0, y, w, y);

		foreachi (vector &pp, mode->param, i){
			complex c = project(pp);
			p->setColor((hover == i) ? Red : Black);
			p->drawCircle(c.x, c.y, 3);
		}

		p->setColor(color(1, 0.7f, 0.7f, 0.7f));
		complex c1 = complex(0,0);
		for (float t=0; t<=1.0f; t += 0.01f){
			complex c2 = project(vector(0, t, mode->inter->get(t)));

			p->drawLine(c1.x, c1.y, c2.x, c2.y);
			c1 = c2;
		}

		//p->drawStr(10, 10, "x");
	}

	virtual void onMouseMove()
	{
		complex m = complex(hui::GetEvent()->mx, hui::GetEvent()->my);
		if (hui::GetEvent()->lbut){
			if (hover >= 0){
				vector v = unproject(m);
				//v.x = mode->param[hover].x;
				mode->param[hover] = v;
				mode->param[0].x = mode->param[0].y = 0;
				mode->param.back().x = mode->param.back().y = 1;
				update();
			}
		}else{
			hover = -1;
			foreachi(vector &pp, mode->param, i){
				complex c = project(pp);
				if ((m - c).abs() < 10)
					hover = i;
			}
			mode->hover = hover;
			redraw("area");
		}
	}

	virtual void onLeftButtonDown()
	{
		complex m = complex(hui::GetEvent()->mx, hui::GetEvent()->my);
		if (hover < 0){
			vector v = unproject(m);
			for (int i=mode->param.num-1; i>=0; i--)
				if (v.x >= mode->param[i].x){
					mode->param.insert(v, i+1);
					hover = i+1;
					break;
				}
			update();

		}
	}


	ModeModelMeshDeformCylinder *mode;
};

 ModeModelMeshDeformCylinder::ModeModelMeshDeformCylinder(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshDeformCylinder", _parent)
{
	geo = NULL;
	has_preview = false;

	param.add(vector(0,0,1));
	param.add(vector(0.5f,0.5f,1));
	param.add(vector(1,1,1));

	inter = new Interpolator<float>(Interpolator<float>::TYPE_CUBIC_SPLINE_NOTANG);

	hover = -1;
	radius = 1;
}

ModeModelMeshDeformCylinder::~ModeModelMeshDeformCylinder()
{
	delete inter;
}

vector get_ev(matrix3 &m)
{
	vector v = e_z;
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

void ModeModelMeshDeformCylinder::onStart()
{
	// Dialog
	dialog = new DeformationCylinderDialog(this);
	dialog->setPositionSpecial(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->event("hui:close", std::bind(&ModeModelMeshDeformCylinder::onClose, this));
	dialog->event("ok", std::bind(&ModeModelMeshDeformCylinder::onOk, this));
	dialog->show();

	multi_view->setAllowAction(false);
	multi_view->setAllowSelect(false);

	foreachi(ModelVertex &v, data->vertex, i)
		if (v.is_selected)
			index.add(i);

	get_axis(data, axis, radius);
	dir = axis[1] - axis[0];
	dir.normalize();

	updateParams();
	hover = -1;
}

void ModeModelMeshDeformCylinder::onEnd()
{
	if (has_preview)
		restore();
	delete(dialog);
	delete(geo);
	multi_view->setAllowAction(true);
	multi_view->setAllowSelect(true);
}

void ModeModelMeshDeformCylinder::updateParams()
{
	inter->clear();
	float last = 0;
	for (vector &pp: param){
		inter->add(pp.z, pp.y - last);
		last = pp.y;
	}

	if (geo)
		delete geo;

	geo = new GeometryCylinder(axis[0], axis[1], radius, CYLINDER_RINGS, CYLINDER_EDGES);
	for (auto &v: geo->vertex)
		v.pos = transform(v.pos);
}

void ModeModelMeshDeformCylinder::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);

	ModeModel::setMaterialCreation();
	geo->build(nix::vb_temp);

	nix::Draw3D(nix::vb_temp);

	nix::line_width = 3;
	nix::SetAlpha(ALPHA_NONE);
	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetColor(Green);
	nix::DrawLine3D(axis[0], axis[1]);

	vector e1 = dir.ortho();
	vector e2 = dir ^ e1;
	foreachi(vector &p, param, ip){
		nix::SetColor((ip == hover) ? Red : Green);
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
	nix::EnableLighting(true);
	nix::line_width = 1;
}

inline bool hover_line(vector &a, vector &b, vector &m, vector &tp)
{
	const float r = 5;
	//if ((a - m).length() < r)
	//	return true;
	if (VecLineDistance(a, b, m) < r){
		vector p = VecLineNearestPoint(m, a, b);
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
{
	if (has_preview)
		restore();
	msg_write("----prev");

	for (ModelVertex &v: geo->vertex)
		v.pos = transform(v.pos);

	for (int vi: index)
		data->vertex[vi].pos = transform(data->vertex[vi].pos);
	data->notify();
	has_preview = true;
	ed->forceRedraw();
}

void ModeModelMeshDeformCylinder::onMouseMove()
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
					param[hover].y = orig_param.y +  lm / la;
					param[hover].x = param[hover].y;
				}
			}
			updateParams();
		}
	}else{
		updateHover();
	}
}

void ModeModelMeshDeformCylinder::onLeftButtonDown()
{
}

void ModeModelMeshDeformCylinder::onLeftButtonUp()
{
}

void ModeModelMeshDeformCylinder::restore()
{
	/*delete(geo);
	vector d = max - min;
	geo = new GeometryCube(min, e_x * d.x, e_y * d.y, e_z * d.z, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
	ed->forceRedraw();

	foreachi(int vi, index, ii)
		data->vertex[vi].pos = old_pos[ii];
	data->notify();

	has_preview = false;*/
}

void ModeModelMeshDeformCylinder::onOk()
{
	if (has_preview)
		restore();

	data->beginActionGroup("deformation");
	for (int vi: index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->vertex[vi].pos)));

	data->endActionGroup();

	abort();
}

void ModeModelMeshDeformCylinder::onClose()
{
	abort();
}
