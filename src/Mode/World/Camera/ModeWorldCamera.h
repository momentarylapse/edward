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
	void OnPointList();
	void OnPointListEdit();
	void OnPointListSelect();
	void OnCamEditVel();
	void OnCloseDialog();

	void LoadData();

	void OnUpdate(Observable *obs);

	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	DataCamera *data;

	CHuiWindow *dialog;

	bool edit_vel;
};

extern ModeWorldCamera *mode_world_camera;

#endif /* MODEWORLDCAMERA_H_ */
