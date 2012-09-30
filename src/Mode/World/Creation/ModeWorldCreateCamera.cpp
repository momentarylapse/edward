/*
 * ModeWorldCreateCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "ModeWorldCreateCamera.h"
#include "../../../Edward.h"
#include "../../../lib/types/interpolation.h"

/*#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11*/

static string cp_type(int type)
{
	if (type == CPKSetCamPos)
		return "SetPos";
	if (type == CPKSetCamPosRel)
		return "SetPosRel";
	if (type == CPKSetCamAng)
		return "SetAng";
	if (type == CPKSetCamPosAng)
		return "SetPosAng";
	if (type == CPKCamFlight)
		return "Flight";
	if (type == CPKCamFlightRel)
		return "FlightRel";
}

ModeWorldCreateCamera::ModeWorldCreateCamera(Mode *_parent, const string &filename) :
	ModeCreation("WorldCreateCamera", _parent)
{
	data = new DataCamera;
	if (filename.num > 0)
		data->Load(filename);

	message = _("Kamera-Fahrt");
}

ModeWorldCreateCamera::~ModeWorldCreateCamera()
{
}

void ModeWorldCreateCamera::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("world_camera_dialog", ed);
	dialog->Update();

	dialog->EventMX("point_list", "hui:activate", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnPointList);
	dialog->EventMX("point_list", "hui:change", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnPointListEdit);
	dialog->EventM("add_point", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnAddPoint);
	dialog->EventM("delete_point", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnDeletePoint);

	dialog->EventM("cam_new", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamNew);
	dialog->EventM("cam_save", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamSave);
	dialog->EventM("cam_save_as", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamSaveAs);

	LoadData();
}

void ModeWorldCreateCamera::OnEnd()
{
	delete(dialog);
	dialog = NULL;
}

void ModeWorldCreateCamera::OnLeftButtonDown()
{
}

void ModeWorldCreateCamera::OnAddPoint()
{
}

void ModeWorldCreateCamera::OnDeletePoint()
{
}

void ModeWorldCreateCamera::OnPointList()
{
}

void ModeWorldCreateCamera::OnPointListEdit()
{
}

void ModeWorldCreateCamera::OnCamNew()
{
}

void ModeWorldCreateCamera::OnCamSave()
{
}

void ModeWorldCreateCamera::OnCamSaveAs()
{
}

void ModeWorldCreateCamera::LoadData()
{
	dialog->Reset("point_list");
	float t0 = 0;
	foreachi(WorldCamPoint &c, data->Point, i){
		dialog->SetString("point_list", format("%d\\%s\\%.3f\\%.3f", i+1, cp_type(c.Type).c_str(), t0, c.Duration));
		t0 += c.Duration;
	}

	PointVel.clear();
	foreach(WorldCamPoint &c, data->Point){
		WorldCamPointVel v;
		v.view_stage = 0;
		v.is_selected = false;
		v.is_special = true;
		v.pos = c.pos + c.Vel;
		PointVel.add(v);
	}


	multi_view->ResetData(data);

	// left -> translate
	/*multi_view->SetMouseAction(0, "ActionWorldMoveSelection", MultiView::ActionMove);
	// middle/right -> rotate
	multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);*/
	multi_view->MVRectable = true;
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->SetData(	MVDWorldCamPoint,
			data->Point,
			NULL,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove | MultiView::FlagDraw,
			NULL, NULL);
	multi_view->SetData(	MVDWorldCamPointVel,
			PointVel,
			NULL,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove | MultiView::FlagDraw,
			NULL, NULL);
}

void ModeWorldCreateCamera::OnDrawWin(int win, irect dest)
{
	NixEnableLighting(false);

	vector last_pos = v_0;
	vector last_vel = v_0;
	foreach(WorldCamPoint &c, data->Point){
		if (c.Type == CPKCamFlight){
			Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE);
			inter.add2(last_pos, last_vel, 0);
			inter.add2(c.pos, c.Vel, c.Duration);
			NixSetColor(White);
			int N = 50;
			for (int n=0;n<N;n++)
				NixDrawLine3D(inter.get((float)n / N), inter.get((float)(n+1) / N));
			NixSetColor(Green);
			NixDrawLine3D(c.pos, c.pos + c.Vel);
		}else{
			NixSetColor(Grey);
			NixDrawLine3D(last_pos, c.pos);
		}
		last_pos = c.pos;
		last_vel = c.Vel;
	}

}
