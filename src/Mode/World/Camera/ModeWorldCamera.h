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
template<class T>
class Interpolator;
class ActionCameraMoveTimeSelection;

class ModeWorldCamera: public Mode<DataCamera>, public HuiEventHandler
{
public:
	ModeWorldCamera(ModeBase *_parent, Data *_data);
	virtual ~ModeWorldCamera();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	void OnAddPoint();
	void OnDeletePoint();
	/*void OnPointList();
	void OnPointListEdit();
	void OnPointListSelect();*/
	void OnAreaDraw();
	void OnAreaLeftButtonDown();
	void OnAreaLeftButtonUp();
	void OnAreaMouseMove();
	void OnAreaMouseWheel();
	void OnCamEditVel();
	void OnCamEditAng();
	void OnCamPreview();
	void OnCamStop();
	void OnCloseDialog();

	void LoadData();

	virtual void OnUpdate(Observable *obs);

	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	void PreviewUpdate();
	void UpdateTimePos();

	HuiWindow *dialog;

	bool edit_vel, edit_ang;
	float time_scale, time_offset;

	int hover;
	bool preview;
	float preview_time;

	Interpolator<vector> *inter_pos, *inter_ang;
	Array<float> time_pos;

	float mt_time0;
	int mouse_distance;
	ActionCameraMoveTimeSelection *mt_action;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
