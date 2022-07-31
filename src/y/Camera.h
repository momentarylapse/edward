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


#include "../lib/base/base.h"
#include "../lib/math/vec3.h"
#include "../lib/math/quaternion.h"
#include "../lib/math/rect.h"
#include "../lib/math/mat4.h"
#include "../y/Component.h"


class Camera : public Component {
public:
	Camera(const rect &dest);
	
	rect dest;

	bool enabled;
	bool show;

	float min_depth, max_depth;
	
	float fov;
	float exposure;
	float bloom_radius;
	float bloom_factor;

	bool focus_enabled;
	float focal_length;
	float focal_blur;

	mat4 view_matrix() const;
	mat4 projection_matrix(float aspect_ratio) const;

	void update_matrices(float aspect_ratio);

	mat4 m_projection, m_view;
	mat4 m_all, im_all;
	vec3 _cdecl project(const vec3 &v);
	vec3 _cdecl unproject(const vec3 &v);

	void _cdecl on_iterate(float dt) override;

	void _cdecl __init__(const vec3 &pos, const quaternion &ang, const rect &dest);
	void _cdecl __delete__() override;

	static const kaba::Class *_class;
};

void CameraInit();
void CameraReset();
void CameraCalcMove(float dt);
void CameraShiftAll(const vec3 &dpos);
Camera *add_camera(const vec3 &pos, const quaternion &ang, const rect &dest);

extern Camera *cam_main; // "camera"


