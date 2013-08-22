/*
 * ModeWorldCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "ModeWorldCamera.h"
#include "../../../Data/World/DataCamera.h"
#include "../../../Edward.h"
#include "../../../lib/math/interpolation.h"
#include "Creation/ModeWorldCameraCreatePoint.h"
#include "../../../Action/World/Camera/ActionCameraMoveTimeSelection.h"

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
	Mode<DataCamera>("WorldCamera", _parent, _data, ed->multi_view_3d, "")
{
	edit_vel = false;
	edit_ang = false;

	time_scale = 10;
	time_offset = 0;

	inter_pos = new Interpolator<vector>(Interpolator<vector>::TYPE_CUBIC_SPLINE);
	inter_ang = new Interpolator<vector>(Interpolator<vector>::TYPE_ANGULAR_LERP);
	mt_action = NULL;
	mouse_distance = -1;
}

ModeWorldCamera::~ModeWorldCamera()
{
	delete(inter_pos);
	delete(inter_ang);
}

void ModeWorldCamera::OnStart()
{
	// Dialog
	dialog = HuiCreateResourceDialog("world_camera_dialog", ed);
	dialog->SetPositionSpecial(ed, HuiBottom);
	dialog->Show();

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

	multi_view->ResetMouseAction();

	Subscribe(data);
	Subscribe(multi_view);
	LoadData();
	hover = -1;
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

	HuiPainter *c = dialog->BeginDraw("cam_area");
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

	float dur = data->GetDuration();
	c->SetColor(color(0.15f, 0, 0, 1));
	c->DrawRect(rect(sample2screen(0), sample2screen(dur), r.y1, r.y2));

	float t0 = 0;
	foreachi(WorldCamPoint &p, data->Point, i){
		c->SetColor(p.is_selected ? Red : Black);
		c->SetLineWidth((i == hover) ? 5.0f : 2.2f);
		c->DrawLine(time_pos[i], r.y1, time_pos[i], r.y2);
		float t1 = t0 + p.Duration;
		t0 = t1;
	}

	Array<float> v;
	const int N = 1000;
	v.resize(N+1);
	float v_max = 0;
	for (int i=0; i<=N; i++){
		float vv = inter_pos->get_tang((float)i / (float)N).length();
		if (vv > v_max)
			v_max = vv;
		v[i] = vv;
	}
	c->SetLineWidth(1.0f);
	c->SetColor(Green);
	for (int i=0; i<N; i++){
		float t0 = (float)i / (float)N * dur;
		float t1 = (float)(i+1) / (float)N * dur;
		c->DrawLine(sample2screen(t0), r.y2 - v[i]/v_max * r.height(), sample2screen(t1), r.y2 - v[i+1]/v_max * r.height());
	}

	if (preview){
		c->SetLineWidth(1.5f);
		c->SetColor(Green);
		c->DrawLine(sample2screen(preview_time), r.y1, sample2screen(preview_time), r.y2);
	}
	c->End();
}

void ModeWorldCamera::OnAreaLeftButtonDown()
{
	if ((hover >= 0) && (data->Point[hover].is_selected)){
		mouse_distance = 0;
		mt_time0 = screen2sample(HuiGetEvent()->mx);
	}else{
		foreachi(WorldCamPoint &p, data->Point, i)
			p.is_selected = (i == hover);
		ed->ForceRedraw();
		dialog->Redraw("cam_area");
	}
}

void ModeWorldCamera::OnAreaLeftButtonUp()
{
	if (mt_action){
		msg_write("mt end");
		mt_action->undo(data);
		data->Execute(mt_action);
		mt_action = NULL;
		mouse_distance = -1;
	}
}

void ModeWorldCamera::OnAreaMouseMove()
{
	int mx = HuiGetEvent()->mx;

	if (HuiGetEvent()->lbut){
		if (mouse_distance >= 0)
			mouse_distance += abs(HuiGetEvent()->dx);
		if (mouse_distance > 5){
			msg_write("mt update");
			if (mt_action){
				mt_action->undo(data);
				delete(mt_action);
			}
			mt_action = new ActionCameraMoveTimeSelection(data, screen2sample(mx), mt_time0);
			mt_action->execute(data);
			data->Notify("Change");
		}
	}else{
		int new_hover = -1;
		foreachi(float t, time_pos, i)
			if (fabs(mx - t) < 5)
				new_hover = i;
		if (new_hover != hover){
			hover = new_hover;
			dialog->Redraw("cam_area");
		}
	}
}

void ModeWorldCamera::OnAreaMouseWheel()
{
	float time_scale_new = min(time_scale * pow(1.1, HuiGetEvent()->dz), 1000);
	time_offset += HuiGetEvent()->mx * (1.0f / time_scale - 1.0f / time_scale_new);
	time_scale = time_scale_new;
	UpdateTimePos();
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
	multi_view->cam.ignore_radius = true;
	HuiRunLaterM(0.020f, (HuiEventHandler*)this, &ModeWorldCamera::PreviewUpdate);
}

void ModeWorldCamera::OnCamStop()
{
	dialog->Enable("cam_stop", false);
	preview = false;
	multi_view->cam.ignore_radius = false;
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
	float duration = data->GetDuration();
	multi_view->cam.pos = inter_pos->get(preview_time / duration);
	multi_view->cam.ang = inter_ang->get(preview_time / duration);

	ed->ForceRedraw();
	if (preview_time > duration)
		OnCamStop();
	if (preview)
		HuiRunLaterM(0.050f, (HuiEventHandler*)this, &ModeWorldCamera::PreviewUpdate);
}

void ModeWorldCamera::UpdateTimePos()
{
	time_pos.clear();
	float t0 = 0;
	bool prev_was_flight = false;
	foreachi(WorldCamPoint &p, data->Point, i){
		float t1 = t0 + p.Duration;
		if (p.Type == CPKCamFlight){
			time_pos.add(sample2screen(t1));
		}else{
			time_pos.add(sample2screen(t0) + (prev_was_flight ? 5 : 0));
		}
		prev_was_flight = (p.Type == CPKCamFlight);
		t0 = t1;
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
	}else{

		dialog->Redraw("cam_area");
	}
}

void ModeWorldCamera::LoadData()
{
	OnUpdateMenu();

	UpdateTimePos();
	dialog->Redraw("cam_area");
	*inter_pos = data->BuildPosInterpolator();
	*inter_ang = data->BuildAngInterpolator();

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
	ed->ForceRedraw();
}

void ModeWorldCamera::OnDrawWin(MultiViewWindow *win)
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

