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
#include "../lib/math/vector.h"
#include "../lib/math/quaternion.h"
#include "../lib/math/rect.h"
#include "../lib/math/matrix.h"
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

	matrix view_matrix() const;
	matrix projection_matrix(float aspect_ratio) const;

	void update_matrices(float aspect_ratio);

	matrix m_projection, m_view;
	matrix m_all, im_all;
	vector _cdecl project(const vector &v);
	vector _cdecl unproject(const vector &v);

	void _cdecl on_iterate(float dt) override;

	void _cdecl __init__(const vector &pos, const quaternion &ang, const rect &dest);
	void _cdecl __delete__() override;

	static const kaba::Class *_class;
};

void CameraInit();
void CameraReset();
void CameraCalcMove(float dt);
void CameraShiftAll(const vector &dpos);
Camera *add_camera(const vector &pos, const quaternion &ang, const rect &dest);

extern Camera *cam; // "camera"
extern Camera *cur_cam; // currently rendering


