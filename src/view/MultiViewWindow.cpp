//
// Created by michi on 11.05.25.
//

#include "MultiViewWindow.h"
#include "MultiView.h"
#include "MultiViewMeshEmitter.h"
#include "Mode.h"
#include "DrawingHelper.h"
#include <renderer/scene/MeshEmitter.h>
#include <renderer/scene/SceneRenderer.h>
#include <renderer/path/RenderPath.h>
#include <y/world/Camera.h>
#include <Session.h>
#include <lib/math/mat3.h>


MultiViewWindow::MultiViewWindow(MultiView* _multi_view) {
	multi_view = _multi_view;
	scene_renderer = new SceneRenderer(RenderPathType::Forward, *multi_view->view_port.scene_view.get());
	scene_renderer->add_emitter(new MultiViewBackgroundEmitter(multi_view));
	scene_renderer->add_emitter(new MultiViewGeometryEmitter(this));
}

vec3 MultiViewWindow::view_pos() const {
	return multi_view->view_port.pos - local_ang * vec3(0, 0, multi_view->view_port.radius);
}

quaternion MultiViewWindow::view_ang() const {
	return local_ang;
}



vec3 MultiViewWindow::project(const vec3& v) const {
	return to_pixels.project(v);
}

vec3 MultiViewWindow::unproject(const vec3& v, const vec3& zref) const {
	vec3 op = project(zref);
	vec3 r = v;
	r.z = op.z;
	return to_pixels.inverse().project(r);
}

vec3 MultiViewWindow::direction() const {
	return multi_view->view_port.ang * vec3::EZ;
}

float MultiViewWindow::zoom() const {

	//return 1000.0f / radius;
	//if (multi_view->mode3d)
		return area.height() / multi_view->view_port.radius;
	//else
	//	return area.height() * 0.8f / multi_view->view_port.radius;
}

#define GRID_CONST	5.0f

float MultiViewWindow::get_grid_d() const {
	return pow(10.0f, ceil(log10(GRID_CONST / zoom())));
}

int grid_level(int i) {
	if (i == 0)
		return 0;
	if (i % 10 == 0)
		return 1;
	if (i % 100 == 0)
		return 2;
	return 3;
}

const float LOW_MAX = 0.25f;
const float MID_MAX = 0.5f;

float grid_density(int level, float d_err) {
	if (level == 0)
		return 1;
	if (level == 1)
		d_err += 1;
	return min((float)pow(10.0f, d_err-1.0f) * LOW_MAX, MID_MAX);
}


int MultiViewWindow::active_grid() const {
	vec3 d = direction();
	d.x = abs(d.x);
	d.y = abs(d.y);
	d.z = abs(d.z);
	if (d.x > d.y and d.x > d.z)
		return 0;
	if (d.y > d.z)
		return 1;
	return 2;
}

vec3 MultiViewWindow::active_grid_direction() const {
	vec3 dd = vec3::EZ;
	int ag = active_grid();
	if (ag == 0)
		dd = vec3::EX;
	if (ag == 1)
		dd = vec3::EY;

	if (dd * direction() < 0)
		return -dd;
	return dd;
}

mat3 MultiViewWindow::active_grid_frame() const {
	const vec3 dir = active_grid_direction();
	const vec3 up = dir.ortho();
	const vec3 right = dir ^ up;
	return {right, up, dir};
}

mat3 MultiViewWindow::edit_frame() const {
	return active_grid_frame();
}

void MultiViewWindow::prepare(const RenderParams& params) {
	projection = multi_view->view_port.cam->projection_matrix(area.width() / area.height());

	scene_renderer->set_view(params, view_pos(), view_ang(), projection);
	view = scene_renderer->rvd.ubo.v;
	scene_renderer->prepare(params);

	// 3d -> pixel
	to_pixels = mat4::translation({area.x1, area.y1, 0})
		* mat4::scale(area.width()/2, area.height()/2, 1)
		* mat4::translation({1.0f, 1.0f, 0})
		* projection * view;
}

void MultiViewWindow::draw(const RenderParams& params) {
	multi_view->session->drawing_helper->set_window(this);

	scene_renderer->draw(params);
}

RenderViewData& MultiViewWindow::rvd() {
	return scene_renderer->rvd;
}


