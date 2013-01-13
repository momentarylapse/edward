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
	edit_ang = false;

	time_scale = 10;
	time_offset = 0;
}

ModeWorldCamera::~ModeWorldCamera()
{
}

void ModeWorldCamera::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("world_camera_dialog", ed);
	dialog->SetPositionSpecial(ed, HuiBottom);
	dialog->Update();

	dialog->EventMX("cam_area", "hui:redraw", this, &ModeWorldCamera::OnAreaDraw);
	dialog->EventMX("cam_area", "hui:left-button-down", this, &ModeWorldCamera::OnAreaLeftButtonDown);
	dialog->EventMX("cam_area", "hui:left-button-up", this, &ModeWorldCamera::OnAreaLeftButtonUp);
	dialog->EventMX("cam_area", "hui:mouse-move", this, &ModeWorldCamera::OnAreaMouseMove);
	dialog->EventMX("cam_area", "hui:mouse-wheel", this, &ModeWorldCamera::OnAreaMouseWheel);
	dialog->EventM("add_point", this, &ModeWorldCamera::OnAddPoint);
	dialog->EventM("delete_point", this, &ModeWorldCamera::OnDeletePoint);
	dialog->EventM("cam_edit_vel", this, &ModeWorldCamera::OnCamEditVel);
	dialog->EventM("cam_edit_ang", this, &ModeWorldCamera::OnCamEditAng);
	dialog->EventM("cam_preview", this, &ModeWorldCamera::OnCamPreview);
	dialog->EventM("cam_stop", this, &ModeWorldCamera::OnCamStop);

	dialog->EventM("hui:close", this, &ModeWorldCamera::OnCloseDialog);

	dialog->Enable("cam_stop", false);



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

#define screen2sample(x)	((x) / time_scale + time_offset)
#define sample2screen(x)	(((x) - time_offset) * time_scale)
string get_time_str_fuzzy(double t, double dt)
{
	bool sign = (t < 0);
	if (sign)
		t = -t;
	int _min=((int)t/60);
	int _sec=((int)t %60);
	int _usec=( ((int)(t * 1000)) %1000);
	if (dt < 1.0){
		if (_min > 0)
			return format("%s%d:%.2d,%.3d",sign?"-":"",_min,_sec,_usec);
		else
			return format("%s%.2d,%.3d",sign?"-":"",_sec,_usec);
	}else{
		if (_min > 0)
			return format("%s%d:%.2d",sign?"-":"",_min,_sec);
		else
			return format("%s%.2d",sign?"-":"",_sec);
	}
}

void ModeWorldCamera::OnAreaDraw()
{
	double MIN_GRID_DIST = 10.0;
	color bg = White;
	color ColorGrid = color(1, 0.75f, 0.75f, 0.75f);

	HuiDrawingContext *c = dialog->BeginDraw("cam_area");
	c->SetLineWidth(0.8f);
	c->SetColor(bg);
	c->DrawRect(0, 0, c->width, c->height);
	c->SetFontSize(8);

	rect r = rect(0, c->width, 0, c->height);
	double dt = MIN_GRID_DIST / time_scale; // >= 10 pixel
	double exp_s = ceil(log10(dt));
	double exp_s_mod = exp_s - log10(dt);
	dt = pow(10, exp_s);
//	double dw = dl * a->view_zoom;
	int nx0 = floor(screen2sample(r.x1 - 1) / dt);
	int nx1 = ceil(screen2sample(r.x2) / dt);
	color c1 = ColorInterpolate(bg, ColorGrid, exp_s_mod);
	color c2 = ColorGrid;
	for (int n=nx0;n<nx1;n++){
		c->SetColor(((n % 10) == 0) ? c2 : c1);
		int xx = sample2screen(n * dt);
		c->DrawLine(xx, 0, xx, c->height);
	}
	c->SetColor(ColorGrid);
	for (int n=nx0;n<nx1;n++){
		if ((sample2screen(dt) - sample2screen(0)) > 30){
			if ((((n % 10) % 3) == 0) && ((n % 10) != 9) && ((n % 10) != -9))
				c->DrawStr(sample2screen(n * dt) + 2, r.y1, get_time_str_fuzzy(n * dt, dt * 3));
		}else{
			if ((n % 10) == 0)
				c->DrawStr(sample2screen(n * dt) + 2, r.y1, get_time_str_fuzzy(n * dt, dt * 10));
		}
	}

	float t0 = 0;
	foreachi(WorldCamPoint &p, data->Point, i){
		c->SetColor(p.is_selected ? Red : Black);
		float t1 = t0 + p.Duration;
		if (p.Type == CPKCamFlight){
			c->DrawLine(sample2screen(t1), r.y1, sample2screen(t1), r.y2);
		}else{
			c->DrawLine(sample2screen(t0), r.y1, sample2screen(t0), r.y2);
			c->SetColor(color(0.2f, 0, 0, 0));
			c->DrawRect(rect(sample2screen(t0), sample2screen(t1), r.y1, r.y2));
		}
		t0 = t1;
	}

	if (preview){
		c->SetColor(Green);
		c->DrawLine(sample2screen(preview_time), r.y1, sample2screen(preview_time), r.y2);
	}
	c->End();
}

void ModeWorldCamera::OnAreaLeftButtonDown()
{
}

void ModeWorldCamera::OnAreaLeftButtonUp()
{
}

void ModeWorldCamera::OnAreaMouseMove()
{
}

void ModeWorldCamera::OnAreaMouseWheel()
{
	float time_scale_new = min(time_scale * pow(1.1, HuiGetEvent()->dz), 1000);
	time_offset += HuiGetEvent()->mx * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	dialog->Redraw("cam_area");
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

void ModeWorldCamera::OnCamEditVel()
{
	edit_vel = dialog->IsChecked("");
	LoadData();
}

void ModeWorldCamera::OnCamEditAng()
{
	edit_ang = dialog->IsChecked("");
	LoadData();
}

void ModeWorldCamera::OnCamPreview()
{
	dialog->Enable("cam_stop", true);
	preview_time = 0;
	preview = true;
	HuiRunLaterM(20, (HuiEventHandler*)this, &ModeWorldCamera::PreviewUpdate);
}

void ModeWorldCamera::OnCamStop()
{
	dialog->Enable("cam_stop", false);
	preview = false;
	multi_view->ignore_radius = false;
	ed->ForceRedraw();
}

void ModeWorldCamera::OnCloseDialog()
{
	if (ed->AllowTermination()){
		New();
		ed->SetMode(parent);
	}
}

void ModeWorldCamera::PreviewUpdate()
{
	preview_time += 0.050f;
	dialog->Redraw("cam_area");
	Interpolator<vector> pos_inter = data->BuildPosInterpolator();
	Interpolator<vector> ang_inter = data->BuildAngInterpolator();
	float duration = data->GetDuration();
	multi_view->pos = pos_inter.get(preview_time / duration);
	multi_view->ang = ang_inter.get(preview_time / duration);
	multi_view->ignore_radius = true;
	ed->ForceRedraw();
	if (preview_time > duration)
		OnCamStop();
	if (preview)
		HuiRunLaterM(50, (HuiEventHandler*)this, &ModeWorldCamera::PreviewUpdate);
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

		dialog->Redraw("cam_area");
	}
}

void ModeWorldCamera::LoadData()
{
	OnUpdateMenu();

	dialog->Redraw("cam_area");


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

