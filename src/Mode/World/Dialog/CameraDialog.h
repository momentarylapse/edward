/*
 * CameraDialog.h
 *
 *  Created on: 21.09.2013
 *      Author: michi
 */

#ifndef CAMERADIALOG_H_
#define CAMERADIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/EmbeddedDialog.h"
#include "../../../Stuff/Observer.h"

class DataCamera;
class ModeWorldCamera;
class ActionCameraMoveTimeSelection;

class CameraDialog: public EmbeddedDialog, public Observer
{
public:
	CameraDialog(HuiWindow *parent, ModeWorldCamera *mode);
	virtual ~CameraDialog();


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

	void UpdateTimePos();

	virtual void onUpdate(Observable *obs);

	DataCamera *data;
	ModeWorldCamera *mode;

	Array<float> time_pos;

	int hover;
	int mouse_distance;

	float time_scale, time_offset;


	float mt_time0;
	ActionCameraMoveTimeSelection *mt_action;
};

#endif /* CAMERADIALOG_H_ */
