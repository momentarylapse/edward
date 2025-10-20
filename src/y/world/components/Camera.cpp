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
#include "../World.h"
#include <ecs/Entity.h>
#include <ecs/ComponentManager.h>
#include <EngineData.h>
#include <lib/math/vec3.h>
#include <lib/math/mat4.h>
#include <lib/yrenderer/scene/CameraParams.h>


const kaba::Class *Camera::_class = nullptr;


#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11



Camera *cam_main = nullptr; // "camera"


void CameraInit() {
	CameraReset();
}

void CameraReset() {
	cam_main = nullptr;
}

Camera::Camera() {
	component_type = _class;

	fov = pi / 4;
	exposure = 1.0f;
	bloom_radius = 10;
	bloom_factor = 0.15f;
	auto_exposure = false;
	auto_exposure_min = 0.3f;
	auto_exposure_max = 4.0f;
	auto_exposure_speed = 1.0f;

	focus_enabled = false;
	focal_length = 2000;
	focal_blur = 2;

	//scale_x = 1;
	//z = 0.999999f;
	min_depth = 1.0f;
	max_depth = 1000000.0f;

	m_all = mat4::ID;
	im_all = mat4::ID;

	enabled = true;
	show = true;
}


//void Camera::__init__(const vec3 &_pos, const quaternion &_ang, const rect &_dest) {
	//new(this) Camera(_pos, _ang, _dest);
//}

void Camera::__delete__() {
	this->Camera::~Camera();
}

// view space -> relative screen space (API independent)
// (-1,-1) = top left
// (+1,+1) = bottom right
mat4 Camera::projection_matrix(float aspect_ratio) const {
	// flip the y-axis
	return mat4::perspective(fov, aspect_ratio, min_depth, max_depth, false) * mat4::scale(1,-1,1);
}

mat4 Camera::view_matrix() const {
	auto o = owner;
	return mat4::rotation(o->ang).transpose() * mat4::translation(-o->pos);
}

void Camera::update_matrix_cache(float aspect_ratio) {
	const auto p = projection_matrix(aspect_ratio);
	const auto v = view_matrix();

	// TODO fix.... use own projection matrix?

	auto m_rel = mat4::translation(vec3(0.5f * engine.physical_aspect_ratio, 0.5f, 0.5f)) * mat4::scale(0.5f * engine.physical_aspect_ratio, 0.5f, 0.5f);

	m_all = m_rel * p * v;
	im_all = m_all.inverse();
}

yrenderer::CameraParams Camera::params() const {
	return {owner->pos, owner->ang, fov, min_depth, max_depth};
}


// into [0:R]x[0:1] system!
vec3 Camera::project(const vec3 &v) {
	return m_all.project(v);
	//return vec3((vv.x * 0.5f + 0.5f) * engine.physical_aspect_ratio, 0.5f + vv.y * 0.5f, vv.z * 0.5f + 0.5f);
}

vec3 Camera::unproject(const vec3 &v) {
	return im_all.project(v);
	/*float xx = (v.x/engine.physical_aspect_ratio - 0.5f) * 2;
	float yy = (v.y - 0.5f) * 2;
	float zz = (v.z - 0.5f) * 2;
	return im_all.project(vec3(xx,yy,zz));*/
}

