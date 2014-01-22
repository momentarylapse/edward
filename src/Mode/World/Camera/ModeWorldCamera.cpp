/*
 * ModeWorldCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "ModeWorldCamera.h"
#include "../Dialog/CameraDialog.h"
#include "../../../Data/World/DataCamera.h"
#include "../../../Edward.h"
#include "../../../lib/math/interpolation.h"
#include "../../../lib/nix/nix.h"
#include "../../../MultiView/MultiView.h"
#include "Creation/ModeWorldCameraCreatePoint.h"

ModeWorldCamera *mode_world_camera = NULL;

/*static string cp_type(int type)
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
}*/

ModeWorldCamera::ModeWorldCamera(ModeBase *_parent, Data *_data) :
	Mode<DataCamera>("WorldCamera", _parent, _data, ed->multi_view_3d, ""),
	Observable("WorldCamera")
{
	edit_vel = false;
	edit_ang = false;


	inter_pos = new Interpolator<vector>(Interpolator<vector>::TYPE_CUBIC_SPLINE);
	inter_ang = new Interpolator<vector>(Interpolator<vector>::TYPE_ANGULAR_LERP);
}

ModeWorldCamera::~ModeWorldCamera()
{
	delete(inter_pos);
	delete(inter_ang);
}

void ModeWorldCamera::OnStart()
{
	dialog = new CameraDialog(ed, this);


	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->AddItem(L("new"),dir + "new.png","cam_new");
	t->AddItem(L("open"),dir + "open.png","cam_open");
	t->AddItem(L("save"),dir + "save.png","cam_save");
	t->AddSeparator();
	t->AddItem(L("undo"),dir + "undo.png","cam_undo");
	t->AddItem(L("redo"),dir + "redo.png","cam_redo");
	t->AddSeparator();
	t->AddItem(_("Push"),dir + "view_push.png","view_push");
	ed->SetTooltip("view_push", _("ViewStage Push"));
	t->AddItem(_("Pop"),dir + "view_pop.png","view_pop");
	ed->SetTooltip("view_pop", _("ViewStage Pop"));
	t->Enable(true);
	t->Configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->Enable(false);

	preview = false;
	preview_time = 0;

	multi_view->ResetMouseAction();

	Observer::Subscribe(data);
	Observer::Subscribe(multi_view);
	LoadData();
}

void ModeWorldCamera::OnEnd()
{
	Observer::Unsubscribe(data);
	Observer::Unsubscribe(multi_view);
	delete(dialog);
	multi_view->ResetData(data);

	parent->OnStart();
}

void ModeWorldCamera::AddPoint()
{
	ed->SetMode(new ModeWorldCameraCreatePoint(ed->cur_mode));
}

void ModeWorldCamera::DeletePoint()
{
}

/*void ModeWorldCamera::OnPointList()
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
}*/

void ModeWorldCamera::SetEditVel(bool edit)
{
	edit_vel = edit;
	LoadData();
	Notify("Change");
}

void ModeWorldCamera::SetEditAng(bool edit)
{
	edit_ang = edit;
	LoadData();
	Notify("Change");
}

void ModeWorldCamera::PreviewStart()
{
	preview_time = 0;
	preview = true;
	multi_view->cam.ignore_radius = true;
	HuiRunLaterM(0.020f, this, &ModeWorldCamera::PreviewUpdate);
	Notify("Change");
}

void ModeWorldCamera::PreviewStop()
{
	preview = false;
	multi_view->cam.ignore_radius = false;
	ed->ForceRedraw();
	Notify("Change");
}

void ModeWorldCamera::PreviewUpdate()
{
	preview_time += 0.050f;
	float duration = data->GetDuration();
	multi_view->cam.pos = inter_pos->get(preview_time / duration);
	multi_view->cam.ang = inter_ang->get(preview_time / duration);

	ed->ForceRedraw();
	if (preview_time > duration)
		PreviewStop();
	if (preview)
		HuiRunLaterM(0.050f, this, &ModeWorldCamera::PreviewUpdate);
	Notify("Change");
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
	}
}

void ModeWorldCamera::LoadData()
{
	OnUpdateMenu();

	*inter_pos = data->BuildPosInterpolator();
	*inter_ang = data->BuildAngInterpolator();

	multi_view->ResetData(data);

	// left -> translate
	multi_view->SetMouseAction("ActionCameraMoveSelection", MultiView::ActionMove);
	// middle/right -> rotate
	/*multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);*/
	multi_view->allow_rect = true;
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->SetData(	MVDWorldCamPoint,
			data->Point,
			NULL,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove | MultiView::FlagDraw);
	if (edit_vel)
	multi_view->SetData(	MVDWorldCamPointVel,
			data->Vel,
			NULL,
			MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove | MultiView::FlagDraw);
	ed->ForceRedraw();
}

void ModeWorldCamera::OnDrawWin(MultiView::Window *win)
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

