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

class ModeWorldCreateTerrain: public ModeCreation<DataWorld> {
public:
	ModeWorldCreateTerrain(ModeBase *_parent);

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void onOk();
	void onClose();
	void onSizeChange();
};

#endif /* MODEWORLDCREATETERRAIN_H_ */
