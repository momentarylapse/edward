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

class ModeModelMeshAutoweld: public ModeCreation<DataModel>
{
public:
	ModeModelMeshAutoweld(ModeBase *_parent);
	virtual ~ModeModelMeshAutoweld();

	virtual void on_start();
	virtual void on_end();

	virtual void on_draw_win(MultiView::Window *win);

	void onSlider();
	void onCancel();
	void onOk();

private:
	float radius;
	float radius_default;
};

#endif /* MODEMODELMESHAUTOWELD_H_ */
