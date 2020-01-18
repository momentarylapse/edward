/*
 * ModeModelMeshCreatePlatonic.h
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPLATONIC_H_
#define MODEMODELMESHCREATEPLATONIC_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"
class Geometry;

class ModeModelMeshCreatePlatonic: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreatePlatonic(ModeBase *parent, int type);
	virtual ~ModeModelMeshCreatePlatonic();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void updateGeometry();

private:
	Geometry *geo;
	int type;
	vector pos;
	float radius;
	bool pos_chosen;
};

#endif /* MODEMODELMESHCREATEPLATONIC_H_ */
