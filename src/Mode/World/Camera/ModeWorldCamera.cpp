/*
 * ModeWorldCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "ModeWorldCamera.h"
#include "../../../Data/World/DataCamera.h"
#include "../../../Edward.h"
#include "../../../lib/types/interpolation.h"
#include "Creation/ModeWorldCameraCreatePoint.h"

ModeWorldCamera *mode_world_camera = NULL;

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

ModeWorldCamera::ModeWorldCamera(Mode *_parent, Data *_data) :
	Mode("WorldCamera", _parent, _data, ed->multi_view_3d, "")
{
	data = (DataCamera*)_data;

	edit_vel = false;
}

ModeWorldCamera::~ModeWorldCamera()
{
}

void ModeWorldCamera::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("world_camera_dialog", ed);
	dialog->Update();

	dialog->EventMX("point_list", "hui:activate", this, &ModeWorldCamera::OnPointList);
	dialog->EventMX("point_list", "hui:change", this, &ModeWorldCamera::OnPointListEdit);
	dialog->EventMX("point_list", "hui:select", this, &ModeWorldCamera::OnPointListSelect);
	dialog->EventM("add_point", this, &ModeWorldCamera::OnAddPoint);
	dialog->EventM("delete_point", this, &ModeWorldCamera::OnDeletePoint);
	dialog->EventM("cam_edit_vel", this, &ModeWorldCamera::OnCamEditVel);

	dialog->EventM("hui:close", this, &ModeWorldCamera::OnCloseDialog);



	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","cam_new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","cam_open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","cam_save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","cam_undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","cam_redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Push"),_("ViewStage Push"),dir + "view_push.png","view_push");
	ed->ToolbarAddItem(_("Pop"),_("ViewStage Pop"),dir + "view_pop.png","view_pop");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);

	multi_view->ResetMouseAction();

	Subscribe(data);
	Subscribe(multi_view);
	LoadData();
}

void ModeWorldCamera::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
	delete(dialog);
	dialog = NULL;
	multi_view->ResetData(data);

	parent->OnStart();
}

void ModeWorldCamera::OnAddPoint()
{
	ed->SetMode(new ModeWorldCameraCreatePoint(ed->cur_mode));
}

void ModeWorldCamera::OnDeletePoint()
{
}

void ModeWorldCamera::OnPointList()
{
}

void ModeWorldCamera::OnPointListEdit()
{
}

void ModeWorldCamera::OnPointListSelect()
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

void ModeWorldCamera::OnCamEditVel()
{
	edit_vel = dialog->IsChecked("");
	LoadData();
}

void ModeWorldCamera::OnCloseDialog()
{
	if (ed->AllowTermination()){
		New();
		ed->SetMode(parent);
	}
}

void ModeWorldCamera::OnCommand(const string &id)
{
	if (id == "cam_undo")
		data->action_manager->Undo();
	if (id == "cam_redo")
		data->action_manager->Redo();

	if (id == "cam_new")
		New();
	if (id == "cam_open")
		Open();
	if (id == "cam_save")
		Save();
	if (id == "cam_save_as")
		SaveAs();
}

void ModeWorldCamera::OnUpdateMenu()
{
	ed->Enable("cam_undo", data->action_manager->Undoable());
	ed->Enable("cam_redo", data->action_manager->Redoable());
}

void ModeWorldCamera::OnUpdate(Observable *obs)
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

void ModeWorldCamera::LoadData()
{
	OnUpdateMenu();

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

void ModeWorldCamera::OnDrawWin(int win)
{
	NixEnableLighting(false);
	NixSetWorldMatrix(m_id);

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

void ModeWorldCamera::New()
{
	if (ed->AllowTermination())
		data->Reset();
}

bool ModeWorldCamera::Open()
{
	if (ed->AllowTermination())
		if (ed->FileDialog(FDCameraFlight, false, true))
			return data->Load(ed->DialogFileComplete);
	return false;
}

bool ModeWorldCamera::Save()
{
	if (data->filename.num > 0)
		return data->Save(data->filename);
	else
		return SaveAs();
}

bool ModeWorldCamera::SaveAs()
{
	if (ed->FileDialog(FDCameraFlight, true, true))
		return data->Save(ed->DialogFileComplete);
	return false;
}

