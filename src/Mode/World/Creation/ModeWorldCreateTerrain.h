/*
 * ModeWorldCreateTerrain.h
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#ifndef MODEWORLDCREATETERRAIN_H_
#define MODEWORLDCREATETERRAIN_H_

#include "../../ModeCreation.h"
class DataWorld;

class ModeWorldCreateTerrain: public ModeCreation<DataWorld>
{
public:
	ModeWorldCreateTerrain(ModeBase *_parent);
	virtual ~ModeWorldCreateTerrain();

	virtual void onStart();
	virtual void onEnd();

	virtual void onDrawWin(MultiView::Window *win);

	void onOk();
	void onClose();
	void onSizeChange();
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
