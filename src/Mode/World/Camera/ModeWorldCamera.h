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

class ModeWorldCamera: public Mode<DataCamera>, public Observable
{
public:
	ModeWorldCamera(ModeBase *_parent, Data *_data);
	virtual ~ModeWorldCamera();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdateMenu();

	virtual void onDrawWin(MultiView::Window *win);

	void addPoint();
	void deletePoint();

	void loadData();

	virtual void onUpdate(Observable *obs, const string &message);

	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	void previewStart();
	void previewStop();
	void setEditAng(bool edit);
	void setEditVel(bool edit);
	void previewUpdate();

	CameraDialog *dialog;

	bool edit_vel, edit_ang;
	Interpolator<vector> *inter_pos, *inter_ang;

	bool preview;
	float preview_time;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
