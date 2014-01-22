/*
 * ActionController.cpp
 *
 *  Created on: 21.01.2014
 *      Author: michi
 */

#include "ActionController.h"
#include "MultiView.h"
#include "MultiViewWindow.h"
#include "../Edward.h"
#include "../lib/nix/nix.h"
#include "../Data/Model/Geometry/Geometry.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryTorus.h"


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


void ActionController::StartAction()
{
	if (cur_action)
		EndAction(false);
	if (!multi_view->allow_mouse_actions)
		return;
	if (action.name != ""){
		//msg_write("mouse action start " + action.name);
		multi_view->MultiViewEditing = true;

		mat = m_id;

		m0 = multi_view->hover.point;
		pos0 = pos;
		if (mode == ActionModeFree)
			pos0 = multi_view->hover.point;
		cur_action = ActionMultiViewFactory(action.name, multi_view->_data_);
		cur_action->execute_logged(multi_view->_data_);
		multi_view->Notify("ActionStart");
	}
}


vector transform_ang(MultiView*mv, const vector &ang)
{
	quaternion qmv, mqmv, qang, q;
	QuaternionRotationV(qmv,  mv->active_win->ang);
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

void ActionController::UpdateAction()
{
	if (!cur_action)
		return;
	//msg_write("mouse action update");

	vector v2p = multi_view->m;
	vector v2  = multi_view->active_win->Unproject(v2p, m0);
	vector v1  = m0;
	vector v1p = multi_view->active_win->Project(v1);
	matrix m_dt, m_dti;
	MatrixTranslation(m_dt, pos0);
	MatrixTranslation(m_dti, -pos0);
	if (action.mode == MultiViewInterface::ActionMove){
		param = mvac_project_trans(mode, v2 - v1);
		MatrixTranslation(mat, param);
	}else if (action.mode == MultiViewInterface::ActionRotate){
		param = mvac_project_trans(mode, v2 - v1) * 0.003f *multi_view->cam.zoom;
		if (mode == ActionModeFree)
			param = transform_ang(multi_view, vector(v1p.y - v2p.y, v1p.x - v2p.x, 0) * 0.003f);
		MatrixRotation(mat, param);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == MultiViewInterface::ActionScale){
		param = vector(1, 1, 1) + mvac_project_trans(mode, v2 - v1) * 0.01f *multi_view->cam.zoom;
		if (mode == ActionModeFree)
			param = vector(1, 1, 1) * (1 + (v2p - v1p).x * 0.01f);
		MatrixScale(mat, param.x, param.y, param.z);
		mat = m_dt * mat * m_dti;
	}else if (action.mode == MultiViewInterface::ActionMirror){
		param = mvac_mirror(mode);
		if (mode == ActionModeFree)
			param = multi_view->active_win->GetDirectionRight();
		plane pl;
		PlaneFromPointNormal(pl, v_0, param);
		MatrixReflect(mat, pl);
		mat = m_dt * mat * m_dti;
	}else{
		param = v_0;
		mat = m_id;
	}
	cur_action->update_and_notify(multi_view->_data_, mat);

	Update();

	multi_view->Notify("ActionUpdate");
}



void ActionController::EndAction(bool set)
{
	if (!cur_action)
		return;
	//msg_write("mouse action end");
	if (set){
		cur_action->undo(multi_view->_data_);
		multi_view->_data_->Execute(cur_action);
		multi_view->Notify("ActionExecute");
	}else{
		cur_action->abort_and_notify(multi_view->_data_);
		delete(cur_action);
		multi_view->Notify("ActionAbort");
	}
	cur_action = NULL;
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

void ActionController::Update()
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
	ed->ForceRedraw();
}

void ActionController::Enable()
{
	Update();
}

void ActionController::Disable()
{
	reset();
	ed->ForceRedraw();
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

void ActionController::Draw(MultiViewWindow *win)
{
	if (!show)
		return;
	NixSetZ(false, false);
	NixEnableLighting(true);
	NixSetWorldMatrix(m_id);
	if (!InUse()){
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

void ActionController::DrawParams()
{
	if (action.mode == MultiViewInterface::ActionMove){
		vector t = param;
		string unit = multi_view->GetMVScaleByZoom(t);
		ed->DrawStr(150, 100, f2s(t.x, 2) + " " + unit, Edward::AlignRight);
		ed->DrawStr(150, 120, f2s(t.y, 2) + " " + unit, Edward::AlignRight);
		if (multi_view->mode3d)
			ed->DrawStr(150, 140, f2s(t.z, 2) + " " + unit, Edward::AlignRight);
	}else if ((action.mode == MultiViewInterface::ActionRotate) or (action.mode == MultiViewInterface::ActionRotate2d)){
		vector r = param * 180.0f / pi;
		ed->DrawStr(150, 100, f2s(r.x, 1) + "°", Edward::AlignRight);
		ed->DrawStr(150, 120, f2s(r.y, 1) + "°", Edward::AlignRight);
		ed->DrawStr(150, 140, f2s(r.z, 1) + "°", Edward::AlignRight);
	}else if ((action.mode == MultiViewInterface::ActionScale) || (action.mode == MultiViewInterface::ActionScale2d)){
		ed->DrawStr(150, 100, f2s(param.x * 100.0f, 1) + "%", Edward::AlignRight);
		ed->DrawStr(150, 120, f2s(param.y * 100.0f, 1) + "%", Edward::AlignRight);
		if (multi_view->mode3d)
			ed->DrawStr(150, 140, f2s(param.z * 100.0f, 1) + "%", Edward::AlignRight);
	}
}

bool ActionController::IsMouseOver(vector &tp)
{
	mouse_over_geo = -1;
	if (!show)
		return false;
	float z_min = 1;
	mouse_over_geo = -1;
	foreachi(Geometry *g, geo, i){
		vector t;
		if (g->IsMouseOver(multi_view->mouse_win, t)){
			float z = multi_view->mouse_win->Project(t).z;
			if ((z < z_min) || (i == 6)){
				mouse_over_geo = i;
				z_min = z;
				tp = t;
			}
		}
	}
	return (mouse_over_geo >= 0);
}

bool ActionController::LeftButtonDown()
{
	if (!show)
		return false;
	vector tp;
	mode = ActionModeNone;
	if (IsMouseOver(multi_view->hover.point)){
		mode = ActionModeX + mouse_over_geo;
		StartAction();
		return true;
	}
	if (multi_view->hover.index >= 0){
		mode = ActionModeFree;
		StartAction();
		return true;
	}
	return false;
}

void ActionController::LeftButtonUp()
{
	EndAction(true);

	Disable();
	if (action.mode > MultiViewInterface::ActionSelect)
		Enable();
}

bool ActionController::InUse()
{
	return cur_action;
}

void ActionController::MouseMove()
{
	UpdateAction();
}
