/*
 * ActionController.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "ActionController.h"
#include "MultiView.h"
#include "MultiViewImpl.h"
#include "Window.h"
#include "../Action/ActionMultiView.h"
#include "../Edward.h"
#include "../lib/nix/nix.h"
#include "../Data/Model/Geometry/Geometry.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryTorus.h"
#include "../Data/Model/Geometry/GeometryCube.h"

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))


namespace MultiView{

ActionController::ActionController(MultiViewImpl *impl)
{
	multi_view = impl;
	geo_mat = m_id;
	mat = m_id;

	reset();
}

void ActionController::startAction(int _constraints)
{
	if (cur_action)
		endAction(false);
	if (!multi_view->allow_mouse_actions)
		return;
	if (action.name == "")
		return;

	mat = m_id;

	m0 = multi_view->hover.point;
	pos0 = pos;
	constraints = _constraints;
	if (constraints == ACTION_CONSTRAINTS_NONE)
		pos0 = multi_view->hover.point;
	cur_action = ActionMultiViewFactory(action.name, data);
	cur_action->execute_logged(data);
	multi_view->notify(multi_view->MESSAGE_ACTION_START);
}


vector transform_ang(MultiViewImpl *mv, const vector &ang)
{
	quaternion qmv, qang, q;
	qmv =  mv->active_win->local_ang;
	QuaternionRotationV(qang, ang);
	q = qmv * qang * qmv.bar();
	return q.get_angles();
}

vector mvac_project_trans(int mode, const vector &v)
{
	vector r = v;
	if (mode == ACTION_CONSTRAINTS_X)
		r.y = r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_Y)
		r.x = r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_Z)
		r.x = r.y = 0;
	else if (mode == ACTION_CONSTRAINTS_XY)
		r.z = 0;
	else if (mode == ACTION_CONSTRAINTS_XZ)
		r.y = 0;
	else if (mode == ACTION_CONSTRAINTS_YZ)
		r.x = 0;
	return r;
}

vector mvac_mirror(int mode)
{
	if (mode == ACTION_CONSTRAINTS_X)
		return e_x;
	else if (mode == ACTION_CONSTRAINTS_Y)
		return e_y;
	else if (mode == ACTION_CONSTRAINTS_Z)
		return e_z;
	else if (mode == ACTION_CONSTRAINTS_XY)
		return e_z;
	else if (mode == ACTION_CONSTRAINTS_XZ)
		return e_y;
	return e_x;
}

void ActionController::updateAction()
{
	if (!cur_action)
		return;

	vector v2p = multi_view->m;
	vector v2  = multi_view->active_win->unproject(v2p, m0);
	vector v1  = m0;
	vector v1p = multi_view->active_win->project(v1);
	vector dir = multi_view->active_win->getDirection();
	matrix m_dt, m_dti;
	MatrixTranslation(m_dt, pos0);
	MatrixTranslation(m_dti, -pos0);
	if (action.mode == ACTION_MOVE){
		param = mvac_project_trans(constraints, v2 - v1);
		MatrixTranslation(mat, param);
	}else if (action.mode == ACTION_ROTATE){
		//param = mvac_project_trans(constraints, v2 - v1) * 0.003f * multi_view->active_win->zoom();
		param = mvac_project_trans(constraints, (v2 - v1) ^ dir) * 0.003f * multi_view->active_win->zoom();
		if (constraints == ACTION_CONSTRAINTS_NONE)
			param = transform_ang(multi_view, vector(v1p.y - v2p.y, v1p.x - v2p.x, 0) * 0.003f);
		MatrixRotation(mat, param);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == ACTION_SCALE){
		param = vector(1, 1, 1) + mvac_project_trans(constraints, v2 - v1) * 0.01f * multi_view->active_win->zoom();
		if (constraints == ACTION_CONSTRAINTS_NONE)
			param = vector(1, 1, 1) * (1 + (v2p - v1p).x * 0.01f);
		MatrixScale(mat, param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == ACTION_MIRROR){
		param = mvac_mirror(constraints);
		if (constraints == ACTION_CONSTRAINTS_NONE)
			param = multi_view->active_win->getDirectionRight();
		plane pl;
		PlaneFromPointNormal(pl, v_0, param);
		MatrixReflect(mat, pl);
		mat = m_dt * mat * m_dti;
	}else{
		param = v_0;
		mat = m_id;
	}
	cur_action->update_and_notify(data, mat);

	update();

	multi_view->notify(multi_view->MESSAGE_ACTION_UPDATE);
}



void ActionController::endAction(bool set)
{
	if (!cur_action)
		return;
	if (set){
		cur_action->undo(data);
		data->execute(cur_action);
		multi_view->notify(multi_view->MESSAGE_ACTION_EXECUTE);
	}else{
		cur_action->abort_and_notify(data);
		delete(cur_action);
		multi_view->notify(multi_view->MESSAGE_ACTION_ABORT);
	}
	cur_action = NULL;
	mat = m_id;
}

bool ActionController::isSelecting()
{
	if (action.mode == ACTION_SELECT)
		return true;
	if (!action.locked)
		return !multi_view->hoverSelected();
	return false;
}

void ActionController::reset()
{
	visible = false;
	constraints = ACTION_CONSTRAINTS_NONE;
	mouse_over_constraint = -1;
	resetGeo();
}

void ActionController::resetGeo()
{
	for (Geometry *g: geo)
		delete(g);
	geo.clear();
	for (Geometry *g: geo_show)
		delete(g);
	geo_show.clear();
}

void ActionController::update()
{
	resetGeo();

	pos = multi_view->getSelectionCenter();
	float f = multi_view->cam.radius * 0.15f;
	if (multi_view->whole_window)
		f /= 2;
	matrix s, t;
	MatrixScale(s, f, f, f);
	MatrixTranslation(t, pos);
	geo_mat = t * s;

	if (visible){
		float r0 = 1.333f;
		float r1 = 0.666f;
		float r = 0.1f;
		geo.add(new GeometryTorus(v_0, e_z, 1.0f, r, 32, 8));
		geo.add(new GeometryTorus(v_0, e_y, 1.0f, r, 32, 8));
		geo.add(new GeometryTorus(v_0, e_x, 1.0f, r, 32, 8));
		geo.add(new GeometryCylinder(-e_x*r0, -e_x*r1, r, 1, 8));
		geo.add(new GeometryCylinder( e_x*r0,  e_x*r1, r, 1, 8));
		geo.add(new GeometryCylinder(-e_y*r0, -e_y*r1, r, 1, 8));
		geo.add(new GeometryCylinder( e_y*r0,  e_y*r1, r, 1, 8));
		geo.add(new GeometryCylinder(-e_z*r0, -e_z*r1, r, 1, 8));
		geo.add(new GeometryCylinder( e_z*r0,  e_z*r1, r, 1, 8));
		r = 0.03f;
		geo_show.add(new GeometryTorus(v_0, e_z, 1.0f, r, 32, 8));
		geo_show.add(new GeometryTorus(v_0, e_y, 1.0f, r, 32, 8));
		geo_show.add(new GeometryTorus(v_0, e_x, 1.0f, r, 32, 8));
		geo_show.add(new GeometryCylinder(-e_x*r0, -e_x*r1, r, 1, 8));
		geo_show.add(new GeometryCylinder( e_x*r0,  e_x*r1, r, 1, 8));
		geo_show.add(new GeometryCylinder(-e_y*r0, -e_y*r1, r, 1, 8));
		geo_show.add(new GeometryCylinder( e_y*r0,  e_y*r1, r, 1, 8));
		geo_show.add(new GeometryCylinder(-e_z*r0, -e_z*r1, r, 1, 8));
		geo_show.add(new GeometryCylinder( e_z*r0,  e_z*r1, r, 1, 8));
	}
	ed->forceRedraw();
}

void ActionController::show(bool show)
{
	visible = show;
	update();
}

struct ACGeoConfig
{
	color col;
	int constraint;
	int priority;
};

string constraint_name(int c)
{
	if (c == ACTION_CONSTRAINTS_X)
		return "X";
	if (c == ACTION_CONSTRAINTS_Y)
		return "Y";
	if (c == ACTION_CONSTRAINTS_Z)
		return "Z";
	if (c == ACTION_CONSTRAINTS_XY)
		return "XY";
	if (c == ACTION_CONSTRAINTS_XZ)
		return "XZ";
	if (c == ACTION_CONSTRAINTS_YZ)
		return "YZ";
	return "free";
}

string action_name(int a)
{
	if (a == ACTION_MOVE)
		return _("move");
	if ((a == ACTION_ROTATE) or (a == ACTION_ROTATE_2D))
		return _("rotate");
	if ((a == ACTION_SCALE) or (a == ACTION_SCALE_2D))
		return _("scale");
	if (a == ACTION_MIRROR)
		return _("reflect");
	return "???";
}

const ACGeoConfig ac_geo_config[] = {
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_XY,0},
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_XZ,0},
	{color(1, 0.4f, 0.4f, 0.8f),ACTION_CONSTRAINTS_YZ,0},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_X,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_X,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Y,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Y,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Z,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_Z,1},
	{color(1, 0.8f, 0.8f, 0.8f),ACTION_CONSTRAINTS_NONE,2}
};

bool geo_allow(int i, Window *win, const matrix &geo_mat)
{
	int c = ac_geo_config[i].constraint;
	vector pp = win->project(geo_mat * v_0);
	vector ppx = win->project(geo_mat * e_x);
	ppx.z = pp.z;
	vector ppy = win->project(geo_mat * e_y);
	ppy.z = pp.z;
	vector ppz = win->project(geo_mat * e_z);
	ppz.z = pp.z;

	if (c == ACTION_CONSTRAINTS_X)
		return (ppx - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_Y)
		return (ppy - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_Z)
		return (ppz - pp).length() > 8;
	if (c == ACTION_CONSTRAINTS_YZ)
		return ((ppy - pp) ^ (ppz - pp)).length() > 300;
	if (c == ACTION_CONSTRAINTS_XZ)
		return ((ppx - pp) ^ (ppz - pp)).length() > 300;
	if (c == ACTION_CONSTRAINTS_XY)
		return ((ppx - pp) ^ (ppy - pp)).length() > 300;
	return true;
}

void ActionController::draw(Window *win)
{
	if (!visible)
		return;
	nix::SetZ(false, false);
	nix::EnableLighting(true);
	matrix m = mat * geo_mat;
	nix::SetWorldMatrix(m);
	nix::SetTexture(NULL);
	nix::SetShader(nix::default_shader_3d);
	foreachi(Geometry *g, geo_show, i){
		if (!geo_allow(i, win, m))
			continue;
		g->build(nix::vb_temp);
		if (ac_geo_config[i].constraint == mouse_over_constraint)
			nix::SetMaterial(Black, White, Black, 0, White);
		else
			nix::SetMaterial(Black, Black, Black, 0, ac_geo_config[i].col);
		nix::Draw3D(nix::vb_temp);
	}
	nix::SetZ(false, false);
	nix::EnableLighting(false);
	nix::SetAlpha(AlphaNone);
	nix::SetWorldMatrix(m_id);

	if (inUse()){
		nix::SetMaterial(White, White, Black, 0, color(1, 0.2f, 0.7f, 0.2f));
		float r = multi_view->cam.radius * 10;
		if (constraints == ACTION_CONSTRAINTS_X)
			nix::DrawLine3D(pos - e_x * r, pos + e_x * r);
		if (constraints == ACTION_CONSTRAINTS_Y)
			nix::DrawLine3D(pos - e_y * r, pos + e_y * r);
		if (constraints == ACTION_CONSTRAINTS_Z)
			nix::DrawLine3D(pos - e_z * r, pos + e_z * r);
	}
	nix::SetMaterial(White, White, Black, 0, Black);

	nix::SetShader(nix::default_shader_2d);

	if (win == multi_view->mouse_win){
		vector pp = win->project(pos);

		if ((mouse_over_constraint >= 0) and !inUse()){
			ed->drawStr(pp.x + 80, pp.y + 40, action_name(action.mode) + " " + constraint_name(mouse_over_constraint));
		}
	}

	if (inUse() and (win == multi_view->active_win)){
		vector pp = win->project(pos);

		float x0 = pp.x + 120;//multi_view->m.x + 100;//win->dest.x1 + 120;
		float y0 = pp.y + 40;//multi_view->m.y + 50;//win->dest.y1 + 100;

		if (action.mode == ACTION_MOVE){
			vector t = param;
			string unit = multi_view->getScaleByZoom(t);
			ed->drawStr(x0, y0,      f2s(t.x, 2) + " " + unit, Edward::ALIGN_RIGHT);
			ed->drawStr(x0, y0 + 20, f2s(t.y, 2) + " " + unit, Edward::ALIGN_RIGHT);
			if (multi_view->mode3d)
				ed->drawStr(x0, y0 + 40, f2s(t.z, 2) + " " + unit, Edward::ALIGN_RIGHT);
		}else if ((action.mode == ACTION_ROTATE) or (action.mode == ACTION_ROTATE_2D)){
			vector r = param * 180.0f / pi;
			ed->drawStr(x0, y0 + 00, f2s(r.x, 1) + "°", Edward::ALIGN_RIGHT);
			ed->drawStr(x0, y0 + 20, f2s(r.y, 1) + "°", Edward::ALIGN_RIGHT);
			ed->drawStr(x0, y0 + 40, f2s(r.z, 1) + "°", Edward::ALIGN_RIGHT);
		}else if ((action.mode == ACTION_SCALE) or (action.mode == ACTION_SCALE_2D)){
			ed->drawStr(x0, y0 + 00, f2s(param.x * 100.0f, 1) + "%", Edward::ALIGN_RIGHT);
			ed->drawStr(x0, y0 + 20, f2s(param.y * 100.0f, 1) + "%", Edward::ALIGN_RIGHT);
			if (multi_view->mode3d)
				ed->drawStr(x0, y0 + 40, f2s(param.z * 100.0f, 1) + "%", Edward::ALIGN_RIGHT);
		}
	}
}

void ActionController::drawPost()
{
}

bool ActionController::isMouseOver(vector &tp)
{
	mouse_over_constraint = -1;
	if (!visible)
		return false;
	float z_min = 1;
	int priority = -1;
	foreachi(Geometry *g, geo, i){
		vector t;
		if (!geo_allow(i, multi_view->mouse_win, geo_mat))
			continue;
		if (g->isMouseOver(multi_view->mouse_win, geo_mat, t)){
			float z = multi_view->mouse_win->project(t).z;
			if ((z < z_min) or (ac_geo_config[i].priority >= priority)){
				mouse_over_constraint = ac_geo_config[i].constraint;
				priority = ac_geo_config[i].priority;
				z_min = z;
				tp = t;
			}
		}
	}
	return (mouse_over_constraint >= 0);
}

bool ActionController::leftButtonDown()
{
	if (!visible and action.locked)
		return false;
	vector tp;
	if (isMouseOver(multi_view->hover.point)){
		startAction(mouse_over_constraint);
		return true;
	}
	if (multi_view->hover.index >= 0){
		startAction(ACTION_CONSTRAINTS_NONE);
		return true;
	}
	return false;
}

void ActionController::leftButtonUp()
{
	endAction(true);

	update();
}

bool ActionController::inUse()
{
	return cur_action;
}

void ActionController::mouseMove()
{
	updateAction();
}

};
