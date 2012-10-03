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
		return _("Sprung");//"SetPosAng";
	if (type == CPKCamFlight)
		return _("Flug");
	if (type == CPKCamFlightRel)
		return "FlightRel";
	return "???";
}

ModeWorldCreateCamera::ModeWorldCreateCamera(Mode *_parent, DataCamera *_data) :
	ModeCreation("WorldCreateCamera", _parent)
{
	data = _data;

	message = _("Kamera-Fahrt");

	edit_vel = false;
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
	dialog->EventMX("point_list", "hui:select", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnPointListSelect);
	dialog->EventM("add_point", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnAddPoint);
	dialog->EventM("delete_point", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnDeletePoint);
	dialog->EventM("cam_edit_vel", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamEditVel);

	dialog->EventM("cam_new", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamNew);
	dialog->EventM("cam_save", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamSave);
	dialog->EventM("cam_save_as", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamSaveAs);
	dialog->EventM("cam_undo", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamUndo);
	dialog->EventM("cam_redo", this, (void(HuiEventHandler::*)())&ModeWorldCreateCamera::OnCamRedo);

	multi_view->ResetMouseAction();

	Subscribe(data);
	Subscribe(multi_view);
	LoadData();
}

void ModeWorldCreateCamera::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	delete(dialog);
	dialog = NULL;
	multi_view->ResetData(data);
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

void ModeWorldCreateCamera::OnPointListSelect()
{
	Array<int> sel = dialog->GetMultiSelection("point_list");
	foreach(WorldCamPoint &c, data->Point)
		c.is_selected = false;
	foreach(WorldCamPointVel &v, data->Vel)
		v.is_selected = false;
	foreach(int i, sel)
		data->Point[i].is_selected = true;
	ed->ForceRedraw();
}

void ModeWorldCreateCamera::OnCamEditVel()
{
	edit_vel = dialog->IsChecked("");
	LoadData();
}

void ModeWorldCreateCamera::OnCamNew()
{	New();	}

void ModeWorldCreateCamera::OnCamSave()
{	Save();	}

void ModeWorldCreateCamera::OnCamSaveAs()
{	SaveAs();	}

void ModeWorldCreateCamera::OnCamUndo()
{
	data->action_manager->Undo();
}

void ModeWorldCreateCamera::OnCamRedo()
{
	data->action_manager->Redo();
}

void ModeWorldCreateCamera::OnUpdate(Observable *obs)
{
	if (obs->GetMessage() == "Change"){
		data->UpdateVel();
		LoadData();
		ed->ForceRedraw();
	}else{

		Array<int> sel;
		foreachi(WorldCamPoint &c, data->Point, i){
			if (c.is_selected)
				sel.add(i);
		}
		dialog->SetMultiSelection("point_list", sel);
	}
}

void ModeWorldCreateCamera::LoadData()
{
	dialog->Enable("cam_undo", data->action_manager->Undoable());
	dialog->Enable("cam_redo", data->action_manager->Redoable());

	dialog->Reset("point_list");
	float t0 = 0;
	Array<int> sel;
	foreachi(WorldCamPoint &c, data->Point, i){
		if (c.is_selected)
			sel.add(i);
		dialog->SetString("point_list", format("%d\\%s\\%.3f\\%.3f", i+1, cp_type(c.Type).c_str(), t0, c.Duration));
		t0 += c.Duration;
	}
	dialog->SetMultiSelection("point_list", sel);


	multi_view->ResetData(data);

	// left -> translate
	multi_view->SetMouseAction(0, "ActionCameraMoveSelection", MultiView::ActionMove);
	// middle/right -> rotate
	/*multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);*/
	multi_view->MVRectable = true;
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->SetData(	MVDWorldCamPoint,
			data->Point,
			NULL,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove | MultiView::FlagDraw,
			NULL, NULL);
	if (edit_vel)
	multi_view->SetData(	MVDWorldCamPointVel,
			data->Vel,
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
			if (edit_vel){
				NixSetColor(Green);
				NixDrawLine3D(c.pos, c.pos + c.Vel);
			}
		}else{
			NixSetColor(Grey);
			NixDrawLine3D(last_pos, c.pos);
		}
		last_pos = c.pos;
		last_vel = c.Vel;
	}

}

void ModeWorldCreateCamera::New()
{
	if (ed->AllowTermination())
		data->Reset();
}

bool ModeWorldCreateCamera::Open()
{
	if (ed->AllowTermination())
		if (ed->FileDialog(FDCameraFlight, false, true))
			return data->Load(ed->DialogFileComplete);
	return false;
}

bool ModeWorldCreateCamera::Save()
{
	if (data->filename.num > 0)
		return data->Save(data->filename);
	else
		return SaveAs();
}

bool ModeWorldCreateCamera::SaveAs()
{
	if (ed->FileDialog(FDCameraFlight, true, true))
		return data->Save(ed->DialogFileComplete);
	return false;
}

