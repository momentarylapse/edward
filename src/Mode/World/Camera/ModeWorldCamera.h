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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	void AddPoint();
	void DeletePoint();

	void LoadData();

	virtual void OnUpdate(Observable *obs);

	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

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
