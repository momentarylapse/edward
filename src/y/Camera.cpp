/*----------------------------------------------------------------------------*\
| Camera                                                                       |
| -> representing the camera (view port)                                       |
| -> can be controlled by a camera script                                      |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2007.12.23 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#include "Camera.h"
#include "Entity3D.h"
#include "World.h"
#include "../y/Entity.h"
#include "../y/ComponentManager.h"
#include "../lib/math/vector.h"
#include "../lib/math/matrix.h"
#include "../y/EngineData.h"


const kaba::Class *Camera::_class = nullptr;


#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11



Camera *cam = nullptr; // "camera"
Camera *cur_cam = nullptr; // currently rendering

Camera *add_camera(const vector &pos, const quaternion &ang, const rect &dest) {
	auto o = world.create_entity(pos, ang);

	auto c = new Camera(dest);
	o->_add_component_external_(c);
	world.register_entity(o);
	return c;
}


void CameraInit() {
	CameraReset();
}

void CameraReset() {
	cam = nullptr;
	cur_cam = cam;
}

Camera::Camera(const rect &_dest) {
	component_type = _class;

	fov = pi / 4;
	exposure = 1.0f;
	bloom_radius = 10;
	bloom_factor = 0.2f;

	focus_enabled = false;
	focal_length = 2000;
	focal_blur = 2;

	//scale_x = 1;
	//z = 0.999999f;
	min_depth = 1.0f;
	max_depth = 1000000.0f;

	m_projection = matrix::ID;
	m_view = matrix::ID;
	m_all = matrix::ID;
	im_all = matrix::ID;

	enabled = true;
	show = true;

	dest = _dest;
}


//void Camera::__init__(const vector &_pos, const quaternion &_ang, const rect &_dest) {
	//new(this) Camera(_pos, _ang, _dest);
//}

void Camera::__delete__() {
	this->Camera::~Camera();
}


void CameraCalcMove(float dt) {
	auto cameras = ComponentManager::get_listx<Camera>();
	for (auto c: *cameras){
		if (!c->enabled)
			continue;
		c->on_iterate(dt);
	}
}

void Camera::on_iterate(float dt) {
}

// view space -> relative screen space (API independent)
// (-1,-1) = top left
// (+1,+1) = bottom right
matrix Camera::projection_matrix(float aspect_ratio) const {
	// flip the y-axis
	return matrix::perspective(fov, aspect_ratio, min_depth, max_depth, false) * matrix::scale(1,-1,1);
}

matrix Camera::view_matrix() const {
	auto o = get_owner<Entity3D>();
	return matrix::rotation_q(o->ang).transpose() * matrix::translation(-o->pos);
}

void Camera::update_matrices(float aspect_ratio) {
	m_projection = projection_matrix(aspect_ratio);
	m_view = view_matrix();

	// TODO fix.... use own projection matrix?

	auto m_rel = matrix::translation(vector(0.5f * engine.physical_aspect_ratio, 0.5f, 0.5f)) * matrix::scale(0.5f * engine.physical_aspect_ratio, 0.5f, 0.5f);

	m_all = m_rel * m_projection * m_view;
	im_all = m_all.inverse();
}

// into [0:R]x[0:1] system!
vector Camera::project(const vector &v) {
	return m_all.project(v);
	//return vector((vv.x * 0.5f + 0.5f) * engine.physical_aspect_ratio, 0.5f + vv.y * 0.5f, vv.z * 0.5f + 0.5f);
}

vector Camera::unproject(const vector &v) {
	return im_all.project(v);
	/*float xx = (v.x/engine.physical_aspect_ratio - 0.5f) * 2;
	float yy = (v.y - 0.5f) * 2;
	float zz = (v.z - 0.5f) * 2;
	return im_all.project(vector(xx,yy,zz));*/
}

void CameraShiftAll(const vector &dpos) {
	auto cameras = ComponentManager::get_listx<Camera>();
	for (auto c: *cameras)
		c->get_owner<Entity3D>()->pos += dpos;
}

