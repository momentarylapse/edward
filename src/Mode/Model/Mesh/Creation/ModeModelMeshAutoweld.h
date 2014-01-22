/*
 * ModeModelMeshAutoweld.h
 *
 *  Created on: 11.11.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHAUTOWELD_H_
#define MODEMODELMESHAUTOWELD_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class ModeModelMeshAutoweld: public ModeCreation<DataModel>, public HuiEventHandler
{
public:
	ModeModelMeshAutoweld(ModeBase *_parent);
	virtual ~ModeModelMeshAutoweld();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnDrawWin(MultiView::Window *win);

	void OnSlider();
	void OnCancel();
	void OnOk();

private:
	float radius;
	float radius_default;
};

#endif /* MODEMODELMESHAUTOWELD_H_ */
