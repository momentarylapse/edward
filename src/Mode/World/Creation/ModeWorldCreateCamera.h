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
	ModeWorldCreateCamera(Mode *_parent, const string &filename);
	virtual ~ModeWorldCreateCamera();

	void OnStart();
	void OnEnd();

	void OnLeftButtonDown();

	void OnDrawWin(int win, irect dest);

	void OnAddPoint();
	void OnDeletePoint();
	void OnPointList();
	void OnPointListEdit();
	void OnCamNew();
	void OnCamSave();
	void OnCamSaveAs();

	void LoadData();

	DataCamera *data;
	Data *GetData(){	return data;	}

	string filename;
	Array<WorldCamPointVel> PointVel;
};

#endif /* MODEWORLDCREATECAMERA_H_ */
