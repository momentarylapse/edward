/*
 * ModeWorldCreateCamera.h
 *
 *  Created on: 30.09.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATECAMERA_H_
#define MODEWORLDCREATECAMERA_H_

#include "../../ModeCreation.h"
#include "../../../Data/World/DataCamera.h"

class ModeWorldCreateCamera: public ModeCreation, public HuiEventHandler
{
public:
	ModeWorldCreateCamera(Mode *_parent, DataCamera *_data);
	virtual ~ModeWorldCreateCamera();

	void OnStart();
	void OnEnd();

	void OnLeftButtonDown();

	void OnDrawWin(int win, irect dest);

	void OnAddPoint();
	void OnDeletePoint();
	void OnPointList();
	void OnPointListEdit();
	void OnPointListSelect();
	void OnCamEditVel();
	void OnCamNew();
	void OnCamSave();
	void OnCamSaveAs();
	void OnCamUndo();
	void OnCamRedo();

	void LoadData();

	void OnUpdate(Observable *obs);

	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	DataCamera *data;
	Data *GetData(){	return data;	}

	bool edit_vel;
	bool adding_point;
};

#endif /* MODEWORLDCREATECAMERA_H_ */
