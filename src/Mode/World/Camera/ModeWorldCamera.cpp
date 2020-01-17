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
	Mode<DataCamera>("WorldCamera", _parent, _data, ed->multi_view_3d, "menu_world"),
	Observable("WorldCamera")
{
	edit_vel = false;
	edit_ang = false;
	dialog = nullptr;
	preview_time = 0;
	preview = false;


	inter_pos = new Interpolator<vector>(Interpolator<vector>::TYPE_CUBIC_SPLINE);
	inter_ang = new Interpolator<vector>(Interpolator<vector>::TYPE_ANGULAR_LERP);
}

ModeWorldCamera::~ModeWorldCamera()
{
	delete(inter_pos);
	delete(inter_ang);
}

void ModeWorldCamera::on_start()
{
	dialog = new CameraDialog(this);
	ed->embed(dialog, "root-table", 0, 1);

	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("world-camera-toolbar");

	auto t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);

	preview = false;
	preview_time = 0;

	multi_view->resetMouseAction();

	Observer::subscribe(data);
	Observer::subscribe(multi_view);
	loadData();
}

void ModeWorldCamera::on_end()
{
	Observer::unsubscribe(data);
	Observer::unsubscribe(multi_view);
	delete(dialog);
	multi_view->clearData(data);
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("world-edit-toolbar"); // ...
}

void ModeWorldCamera::addPoint()
{
	ed->set_mode(new ModeWorldCameraCreatePoint(ed->cur_mode));
}

void ModeWorldCamera::deletePoint()
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

void ModeWorldCamera::setEditVel(bool edit)
{
	edit_vel = edit;
	loadData();
	notify();
}

void ModeWorldCamera::setEditAng(bool edit)
{
	edit_ang = edit;
	loadData();
	notify();
}

void ModeWorldCamera::previewStart()
{
	preview_time = 0;
	preview = true;
	multi_view->cam.ignore_radius = true;
	hui::RunLater(0.020f, std::bind(&ModeWorldCamera::previewUpdate, this));
	notify();
}

void ModeWorldCamera::previewStop()
{
	preview = false;
	multi_view->cam.ignore_radius = false;
	ed->force_redraw();
	notify();
}

void ModeWorldCamera::previewUpdate()
{
	preview_time += 0.050f;
	float duration = data->GetDuration();
	multi_view->cam.pos = inter_pos->get(preview_time / duration);
	multi_view->cam.ang = quaternion::rotation_v(inter_ang->get(preview_time / duration));

	ed->force_redraw();
	if (preview_time > duration)
		previewStop();
	if (preview)
		hui::RunLater(0.050f, std::bind(&ModeWorldCamera::previewUpdate, this));
	notify();
}

void ModeWorldCamera::on_command(const string &id)
{
	if (id == "cam_undo")
		data->action_manager->undo();
	if (id == "cam_redo")
		data->action_manager->redo();

	if (id == "cam_new")
		_new();
	if (id == "cam_open")
		open();
	if (id == "cam_save")
		save();
	if (id == "cam_save_as")
		save_as();
}

void ModeWorldCamera::on_update_menu()
{
	ed->enable("cam_undo", data->action_manager->undoable());
	ed->enable("cam_redo", data->action_manager->redoable());
}

void ModeWorldCamera::on_update(Observable *o, const string &message)
{
	if (message == data->MESSAGE_CHANGE){
		data->UpdateVel();
		loadData();
	}
}

void ModeWorldCamera::loadData()
{
	on_update_menu();

	*inter_pos = data->BuildPosInterpolator();
	*inter_ang = data->BuildAngInterpolator();

	multi_view->clearData(data);

	// left -> translate
	multi_view->setMouseAction("ActionCameraMoveSelection", MultiView::ACTION_MOVE, true);
	// middle/right -> rotate
	/*multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);*/
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->addData(	MVD_WORLD_CAM_POINT,
			data->Point,
			NULL,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	if (edit_vel)
	multi_view->addData(	MVD_WORLD_CAM_POINT_VEL,
			data->Vel,
			NULL,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	ed->force_redraw();
}

void ModeWorldCamera::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	nix::EnableLighting(false);
	nix::SetWorldMatrix(matrix::ID);

	vector last_pos = v_0;
	vector last_vel = v_0;
	for (WorldCamPoint &c: data->Point){
		if (c.Type == CPKCamFlight){
			Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE);
			inter.add2(last_pos, last_vel, 0);
			inter.add2(c.pos, c.Vel, c.Duration);
			inter.normalize();
			nix::SetColor(White);
			int N = 50;
			for (int n=0;n<N;n++)
				nix::DrawLine3D(inter.get((float)n / N), inter.get((float)(n+1) / N));
			if (edit_vel){
				nix::SetColor(Green);
				nix::DrawLine3D(c.pos, c.pos + c.Vel);
			}
		}else{
			nix::SetColor(Grey);
			nix::DrawLine3D(last_pos, c.pos);
		}
		last_pos = c.pos;
		last_vel = c.Vel;
	}

}

void ModeWorldCamera::_new()
{
	if (ed->allow_termination())
		data->reset();
}

bool ModeWorldCamera::open()
{
	if (ed->allow_termination())
		if (ed->file_dialog(FD_CAMERAFLIGHT, false, true))
			return data->load(ed->dialog_file_complete);
	return false;
}

bool ModeWorldCamera::save()
{
	if (data->filename.num > 0)
		return data->save(data->filename);
	else
		return save_as();
}

bool ModeWorldCamera::save_as()
{
	if (ed->file_dialog(FD_CAMERAFLIGHT, true, true))
		return data->save(ed->dialog_file_complete);
	return false;
}

