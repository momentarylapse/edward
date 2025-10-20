/*----------------------------------------------------------------------------*\
| Camera                                                                       |
| -> representing the camera (view port)                                       |
| -> can be controlled by a camera script                                      |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last update: 2009.11.22 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/
#pragma once


#include <lib/base/base.h>
#include <lib/math/vec3.h>
#include <lib/math/quaternion.h>
#include <lib/math/rect.h>
#include <lib/math/mat4.h>
#include <ecs/Component.h>

namespace yrenderer {
	struct CameraParams;
}


class Camera : public Component {
public:
	Camera();

	bool enabled;
	bool show;

	float min_depth, max_depth;
	
	float fov;
	float exposure;
	float bloom_radius;
	float bloom_factor;

	bool auto_exposure;
	float auto_exposure_min;
	float auto_exposure_max;
	float auto_exposure_speed;

	bool focus_enabled;
	float focal_length;
	float focal_blur;

	mat4 view_matrix() const;
	mat4 projection_matrix(float aspect_ratio) const;

	void update_matrix_cache(float aspect_ratio);

	yrenderer::CameraParams params() const;

	mat4 m_all, im_all;
	vec3 _cdecl project(const vec3 &v);
	vec3 _cdecl unproject(const vec3 &v);

	void _cdecl __init__(const vec3 &pos, const quaternion &ang, const rect &dest);
	void _cdecl __delete__() override;

	static const kaba::Class *_class;
};

void CameraInit();
void CameraReset();

extern Camera *cam_main; // "camera"


