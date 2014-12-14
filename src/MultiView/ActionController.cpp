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

#define MVGetSingleData(d, index)	((SingleData*) ((char*)(d).data->data + (d).data->element_size* index))

enum{
	ActionModeNone,
	ActionModeX,
	ActionModeY,
	ActionModeZ,
	ActionModeXY,
	ActionModeXZ,
	ActionModeYZ,
	ActionModeFree,
};

namespace MultiView{

ActionController::ActionController(MultiViewImpl *impl)
{
	multi_view = impl;
}

void ActionController::startAction()
{
	if (cur_action)
		endAction(false);
	if (!multi_view->allow_mouse_actions)
		return;
	if (action.name != ""){
		//msg_write("mouse action start " + action.name);

		mat = m_id;

		m0 = multi_view->hover.point;
		pos0 = pos;
		if (mode == ActionModeFree)
			pos0 = multi_view->hover.point;
		cur_action = ActionMultiViewFactory(action.name, data);
		cur_action->execute_logged(data);
		multi_view->notify(multi_view->MESSAGE_ACTION_START);
	}
}


vector transform_ang(MultiViewImpl *mv, const vector &ang)
{
	quaternion qmv, mqmv, qang, q;
	qmv =  mv->active_win->ang;
	QuaternionRotationV(qang, ang);
	mqmv = qmv;
	mqmv.invert();
	q = qang * mqmv;
	q = qmv * q;
	return q.get_angles();
}

vector mvac_project_trans(int mode, const vector &v)
{
	vector r = v;
	if (mode == ActionModeX)
		r.y = r.z = 0;
	else if (mode == ActionModeY)
		r.x = r.z = 0;
	else if (mode == ActionModeZ)
		r.x = r.y = 0;
	else if (mode == ActionModeXY)
		r.z = 0;
	else if (mode == ActionModeXZ)
		r.y = 0;
	else if (mode == ActionModeYZ)
		r.x = 0;
	return r;
}

vector mvac_mirror(int mode)
{
	if (mode == ActionModeX)
		return e_x;
	else if (mode == ActionModeY)
		return e_y;
	else if (mode == ActionModeZ)
		return e_z;
	else if (mode == ActionModeXY)
		return e_z;
	else if (mode == ActionModeXZ)
		return e_y;
	return e_x;
}

void ActionController::updateAction()
{
	if (!cur_action)
		return;
	//msg_write("mouse action update");

	vector v2p = multi_view->m;
	vector v2  = multi_view->active_win->unproject(v2p, m0);
	vector v1  = m0;
	vector v1p = multi_view->active_win->project(v1);
	matrix m_dt, m_dti;
	MatrixTranslation(m_dt, pos0);
	MatrixTranslation(m_dti, -pos0);
	if ((action.mode == ActionMove) || (action.mode == ActionSelectAndMove)){
		param = mvac_project_trans(mode, v2 - v1);
		MatrixTranslation(mat, param);
	}else if (action.mode == ActionRotate){
		param = mvac_project_trans(mode, v2 - v1) * 0.003f *multi_view->cam.zoom;
		if (mode == ActionModeFree)
			param = transform_ang(multi_view, vector(v1p.y - v2p.y, v1p.x - v2p.x, 0) * 0.003f);
		MatrixRotation(mat, param);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == ActionScale){
		param = vector(1, 1, 1) + mvac_project_trans(mode, v2 - v1) * 0.01f *multi_view->cam.zoom;
		if (mode == ActionModeFree)
			param = vector(1, 1, 1) * (1 + (v2p - v1p).x * 0.01f);
		MatrixScale(mat, param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == ActionMirror){
		param = mvac_mirror(mode);
		if (mode == ActionModeFree)
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
	//msg_write("mouse action end");
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
}

bool ActionController::isSelecting()
{
	if (action.mode == ActionSelect)
		return true;
	if (action.mode == ActionSelectAndMove){
		if (multi_view->hover.index >= 0){
			SingleData *d = MVGetSingleData(multi_view->data[multi_view->hover.set], multi_view->hover.index);
			return !d->is_selected;
		}
		return true;
	}
	return false;
}

void ActionController::reset()
{
	show = false;
	mode = ActionModeNone;
	foreach(Geometry *g, geo)
		delete(g);
	geo.clear();
	foreach(Geometry *g, geo_show)
		delete(g);
	geo_show.clear();
}

void ActionController::update()
{
	int m = mode;
	reset();
	mode = m;
	if (multi_view->HasSelection()){
		pos = multi_view->GetSelectionCenter();
		show = true;
		float f = multi_view->cam.radius * 0.1f;
		matrix s, t;
		MatrixScale(s, f, f, f);
		MatrixTranslation(t, pos);
		geo.add(new GeometryCylinder(-e_x, e_x, 0.1f, 1, 8, false));
		geo.add(new GeometryCylinder(-e_y, e_y, 0.1f, 1, 8, false));
		geo.add(new GeometryCylinder(-e_z, e_z, 0.1f, 1, 8, false));
		geo.add(new GeometryTorus(v_0, e_z, 0.5f, 0.1f, 32, 8));
		geo.add(new GeometryTorus(v_0, e_y, 0.5f, 0.1f, 32, 8));
		geo.add(new GeometryTorus(v_0, e_x, 0.5f, 0.1f, 32, 8));
		geo.add(new GeometryBall(v_0, 0.3f, 16, 8));
		geo_show.add(new GeometryCylinder(-e_x, e_x, 0.05f, 1, 8, false));
		geo_show.add(new GeometryCylinder(-e_y, e_y, 0.05f, 1, 8, false));
		geo_show.add(new GeometryCylinder(-e_z, e_z, 0.05f, 1, 8, false));
		geo_show.add(new GeometryTorus(v_0, e_z, 0.5f, 0.05f, 32, 8));
		geo_show.add(new GeometryTorus(v_0, e_y, 0.5f, 0.05f, 32, 8));
		geo_show.add(new GeometryTorus(v_0, e_x, 0.5f, 0.05f, 32, 8));
		geo_show.add(new GeometryBall(v_0, 0.25f, 16, 8));
		foreach(Geometry *g, geo)
			g->Transform(t * s);
		foreach(Geometry *g, geo_show)
			g->Transform(t * s);
	}
	ed->forceRedraw();
}

void ActionController::enable()
{
	update();
}

void ActionController::disable()
{
	reset();
	ed->forceRedraw();
}

const color MVACColor[] = {
	color(1, 0.8f, 0.8f, 0.8f),
	color(1, 0.8f, 0.8f, 0.8f),
	color(1, 0.8f, 0.8f, 0.8f),
	color(1, 0.4f, 0.4f, 0.8f),
	color(1, 0.4f, 0.4f, 0.8f),
	color(1, 0.4f, 0.4f, 0.8f),
	color(1, 0.8f, 0.8f, 0.8f)
};

void ActionController::draw(Window *win)
{
	if (!show)
		return;
	NixSetZ(false, false);
	NixEnableLighting(true);
	NixSetWorldMatrix(m_id);
	if (!inUse()){
		foreachi(Geometry *g, geo_show, i){
			g->Preview(VBTemp);
			NixSetMaterial(Black, Black, Black, 0, MVACColor[i]);
			NixDraw3D(VBTemp);
		}
	}
	if (mouse_over_geo >= 0){
		NixSetAlpha(AlphaMaterial);
		NixSetMaterial(Black, color(0.8f, 0, 0, 0), Black, 0, White);
		geo_show[mouse_over_geo]->Preview(VBTemp);
		NixDraw3D(VBTemp);
	}
	NixSetZ(false, false);
	NixEnableLighting(false);
	NixSetAlpha(AlphaNone);
	NixSetMaterial(White, White, Black, 0, Black);
	/*NixSetColor(Red);
	vector p = win->Project(pos);
	NixDrawRect(p.x-15, p.x+15, p.y-15, p.y+15, 0);*/
}

void ActionController::drawParams()
{
	if ((action.mode == ActionMove) || (action.mode == ActionSelectAndMove)){
		vector t = param;
		string unit = multi_view->GetMVScaleByZoom(t);
		ed->drawStr(150, 100, f2s(t.x, 2) + " " + unit, Edward::AlignRight);
		ed->drawStr(150, 120, f2s(t.y, 2) + " " + unit, Edward::AlignRight);
		if (multi_view->mode3d)
			ed->drawStr(150, 140, f2s(t.z, 2) + " " + unit, Edward::AlignRight);
	}else if ((action.mode == ActionRotate) or (action.mode == ActionRotate2d)){
		vector r = param * 180.0f / pi;
		ed->drawStr(150, 100, f2s(r.x, 1) + "°", Edward::AlignRight);
		ed->drawStr(150, 120, f2s(r.y, 1) + "°", Edward::AlignRight);
		ed->drawStr(150, 140, f2s(r.z, 1) + "°", Edward::AlignRight);
	}else if ((action.mode == ActionScale) || (action.mode == ActionScale2d)){
		ed->drawStr(150, 100, f2s(param.x * 100.0f, 1) + "%", Edward::AlignRight);
		ed->drawStr(150, 120, f2s(param.y * 100.0f, 1) + "%", Edward::AlignRight);
		if (multi_view->mode3d)
			ed->drawStr(150, 140, f2s(param.z * 100.0f, 1) + "%", Edward::AlignRight);
	}
}

bool ActionController::isMouseOver(vector &tp)
{
	mouse_over_geo = -1;
	if (!show)
		return false;
	float z_min = 1;
	mouse_over_geo = -1;
	foreachi(Geometry *g, geo, i){
		vector t;
		if (g->IsMouseOver(multi_view->mouse_win, t)){
			float z = multi_view->mouse_win->project(t).z;
			if ((z < z_min) || (i == 6)){
				mouse_over_geo = i;
				z_min = z;
				tp = t;
			}
		}
	}
	return (mouse_over_geo >= 0);
}

bool ActionController::leftButtonDown()
{
	if ((!show) && (action.mode != ActionSelectAndMove))
		return false;
	vector tp;
	mode = ActionModeNone;
	if (isMouseOver(multi_view->hover.point)){
		mode = ActionModeX + mouse_over_geo;
		startAction();
		return true;
	}
	if (multi_view->hover.index >= 0){
		mode = ActionModeFree;
		startAction();
		return true;
	}
	return false;
}

void ActionController::leftButtonUp()
{
	endAction(true);
	bool _show = show;

	disable();
	if (_show)
		enable();
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
