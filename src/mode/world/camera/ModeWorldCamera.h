/*
 * ModeWorldCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef MODEWORLDCAMERA_H_
#define MODEWORLDCAMERA_H_

#include "../../Mode.h"
namespace hui{
	class Window;
}
class DataCamera;
class CameraDialog;

template<class T>
class Interpolator;

class ModeWorldCamera: public Mode<DataCamera> {
public:
	ModeWorldCamera(ModeBase *_parent, Data *_data);
	virtual ~ModeWorldCamera();

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;
	void on_update_menu() override;

	void on_draw_win(MultiView::Window *win) override;

	void addPoint();
	void deletePoint();

	void loadData();

	void _new();
	void open();
	void save();
	void save_as();

	void previewStart();
	void previewStop();
	void setEditAng(bool edit);
	void setEditVel(bool edit);
	void previewUpdate();

	CameraDialog *dialog;

	bool edit_vel, edit_ang;
	Interpolator<vec3> *inter_pos, *inter_ang;

	bool preview;
	float preview_time;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
