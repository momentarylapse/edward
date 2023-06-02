/*
 * ModeModelMeshCreateBall.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEBALL_H_
#define MODEMODELMESHCREATEBALL_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class Geometry;

class ModeModelMeshCreateBall: public ModeCreation<DataModel>
{
public:
	ModeModelMeshCreateBall(ModeBase *_parent);
	virtual ~ModeModelMeshCreateBall();

	void on_start() override;
	void on_end() override;

	void on_mouse_move() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void updateGeometry();
	void onTypeBall();
	void onTypeSphere();
	void onTypePhysical();

private:
	vec3 pos;
	float radius;
	bool pos_chosen;
	Geometry *geo;
};

#endif /* MODEMODELMESHCREATEBALL_H_ */
