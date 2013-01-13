/*
 * ModeWorldCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef MODEWORLDCAMERA_H_
#define MODEWORLDCAMERA_H_

#include "../../Mode.h"
class CHuiWindow;
class DataCamera;

class ModeWorldCamera: public Mode, public HuiEventHandler
{
public:
	ModeWorldCamera(Mode *_parent, Data *_data);
	virtual ~ModeWorldCamera();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdateMenu();

	void OnDrawWin(int win);

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

	void OnUpdate(Observable *obs);

	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void PreviewUpdate();

	DataCamera *data;

	CHuiWindow *dialog;

	bool edit_vel, edit_ang;
	float time_scale, time_offset;

	bool preview;
	float preview_time;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
