/*
 * ModeWorldCamera.cpp
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#include "ModeWorldCamera.h"
#include "creation/ModeWorldCameraCreatePoint.h"
#include "../ModeWorld.h"
#include "../dialog/CameraDialog.h"
#include "../../../data/world/DataCamera.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include "../../../lib/math/interpolation.h"
#include "../../../lib/nix/nix.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/Window.h"
#include "../../../multiview/DrawingHelper.h"

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
		return _("Jump");//"SetPosAng";
	if (type == CPKCamFlight)
		return _("Flight");
	if (type == CPKCamFlightRel)
		return "FlightRel";
	return "???";
}*/

ModeWorldCamera::ModeWorldCamera(ModeWorld *_parent, Data *_data) :
	Mode<ModeWorld, DataCamera>(_parent->session, "WorldCamera", _parent, _data, _parent->multi_view, "menu_world")
{
	edit_vel = false;
	edit_ang = false;
	dialog = nullptr;
	preview_time = 0;
	preview = false;


	inter_pos = new Interpolator<vec3>(Interpolator<vec3>::TYPE_CUBIC_SPLINE);
	inter_ang = new Interpolator<vec3>(Interpolator<vec3>::TYPE_ANGULAR_LERP);
}

ModeWorldCamera::~ModeWorldCamera() {
	delete inter_pos;
	delete inter_ang;
}

void ModeWorldCamera::on_start() {
	// timeline....
	dialog = new CameraDialog(this);
	session->win->set_bottom_panel(dialog);

	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("world-camera-toolbar");

	auto t = session->win->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);

	preview = false;
	preview_time = 0;

	multi_view->reset_mouse_action();

	data->out_changed >> create_sink([=]{
		data->UpdateVel();
		loadData();
	});
	loadData();
}

void ModeWorldCamera::on_end() {
	data->unsubscribe(this);
	session->win->set_bottom_panel(nullptr);

	multi_view->clear_data(data);
	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("world-edit-toolbar"); // ...
}

void ModeWorldCamera::addPoint() {
	session->set_mode(new ModeWorldCameraCreatePoint(this));
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
	//notify();
}

void ModeWorldCamera::setEditAng(bool edit)
{
	edit_ang = edit;
	loadData();
	//notify();
}

void ModeWorldCamera::previewStart()
{
	preview_time = 0;
	preview = true;
	multi_view->cam.ignore_radius = true;
	hui::run_later(0.020f, [=]{ previewUpdate(); });
	//notify();
}

void ModeWorldCamera::previewStop()
{
	preview = false;
	multi_view->cam.ignore_radius = false;
	multi_view->force_redraw();
	//notify();
}

void ModeWorldCamera::previewUpdate()
{
	preview_time += 0.050f;
	float duration = data->GetDuration();
	multi_view->cam.pos = inter_pos->get(preview_time / duration);
	multi_view->cam.ang = quaternion::rotation_v(inter_ang->get(preview_time / duration));

	multi_view->force_redraw();
	if (preview_time > duration)
		previewStop();
	if (preview)
		hui::run_later(0.050f, [=]{ previewUpdate(); });
	//notify();
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
	session->win->enable("cam_undo", data->action_manager->undoable());
	session->win->enable("cam_redo", data->action_manager->redoable());
}

void ModeWorldCamera::loadData()
{
	on_update_menu();

	*inter_pos = data->BuildPosInterpolator();
	*inter_ang = data->BuildAngInterpolator();

	multi_view->clear_data(data);

	// left -> translate
	multi_view->set_mouse_action("ActionCameraMoveSelection", MultiView::ACTION_MOVE, true);
	// middle/right -> rotate
	/*multi_view->SetMouseAction(1, "ActionWorldRotateObjects", MultiView::ActionRotate2d);
	multi_view->SetMouseAction(2, "ActionWorldRotateObjects", MultiView::ActionRotate);*/
	//CModeAll::SetMultiViewViewStage(&ViewStage, false);
	multi_view->add_data(MVD_WORLD_CAM_POINT,
			data->Point,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	if (edit_vel)
	multi_view->add_data(MVD_WORLD_CAM_POINT_VEL,
			data->Vel,
			MultiView::FLAG_INDEX | MultiView::FLAG_SELECT | MultiView::FLAG_MOVE | MultiView::FLAG_DRAW);
	multi_view->force_redraw();
}

void ModeWorldCamera::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

#if HAS_LIB_GL
	nix::set_model_matrix(mat4::ID);
#endif
	win->drawing_helper->set_line_width(1.0f);

	vec3 last_pos = v_0;
	vec3 last_vel = v_0;
	for (WorldCamPoint &c: data->Point){
		if (c.Type == CPKCamFlight){
			Interpolator<vec3> inter(Interpolator<vec3>::TYPE_CUBIC_SPLINE);
			inter.add2(last_pos, last_vel, 0);
			inter.add2(c.pos, c.Vel, c.Duration);
			inter.normalize();
			win->drawing_helper->set_color(White);
			int N = 50;
			for (int n=0;n<N;n++)
				win->drawing_helper->draw_line(inter.get((float)n / N), inter.get((float)(n+1) / N));
			if (edit_vel){
				win->drawing_helper->set_color(Green);
				win->drawing_helper->draw_line(c.pos, c.pos + c.Vel);
			}
		}else{
			win->drawing_helper->set_color(Grey);
			win->drawing_helper->draw_line(last_pos, c.pos);
		}
		last_pos = c.pos;
		last_vel = c.Vel;
	}

}

void ModeWorldCamera::_new() {
	session->allow_termination().then([this] {
		data->reset();
	});
}

void ModeWorldCamera::open() {
	session->allow_termination().then([this] {
		session->storage->file_dialog(FD_CAMERAFLIGHT, false, true).then([this] (const auto& p) {
			data->load(p.complete);
		});
	});
}

void ModeWorldCamera::save() {
	if (data->filename)
		save_as();
	else
		data->save(data->filename);
}

void ModeWorldCamera::save_as() {
	session->storage->file_dialog(FD_CAMERAFLIGHT, true, true).then([this] (const auto& p) {
		return data->save(p.complete);
	});
}

