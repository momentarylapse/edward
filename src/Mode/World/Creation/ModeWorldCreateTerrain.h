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

	virtual void on_start();
	virtual void on_end();

	virtual void on_draw_win(MultiView::Window *win);

	void onOk();
	void onClose();
	void onSizeChange();
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
