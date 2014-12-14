/*
 * ModeWorldCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef MODEWORLDCAMERA_H_
#define MODEWORLDCAMERA_H_

#include "../../Mode.h"
class HuiWindow;
class DataCamera;
class CameraDialog;

template<class T>
class Interpolator;

class ModeWorldCamera: public Mode<DataCamera>, public HuiEventHandler, public Observable
{
public:
	ModeWorldCamera(ModeBase *_parent, Data *_data);
	virtual ~ModeWorldCamera();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdateMenu();

	virtual void onDrawWin(MultiView::Window *win);

	void AddPoint();
	void DeletePoint();

	void LoadData();

	virtual void onUpdate(Observable *obs);

	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

	void PreviewStart();
	void PreviewStop();
	void SetEditAng(bool edit);
	void SetEditVel(bool edit);
	void PreviewUpdate();

	CameraDialog *dialog;

	bool edit_vel, edit_ang;
	Interpolator<vector> *inter_pos, *inter_ang;

	bool preview;
	float preview_time;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
