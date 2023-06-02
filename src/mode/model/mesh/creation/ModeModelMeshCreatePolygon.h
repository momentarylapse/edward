/*
 * ModeModelMeshCreatePolygon.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHCREATEPOLYGON_H_
#define MODEMODELMESHCREATEPOLYGON_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelMeshCreatePolygon: public ModeCreation<DataModel> {
public:
	ModeModelMeshCreatePolygon(ModeBase *parent);

	void on_start() override;
	void on_end() override;

	void on_left_button_down() override;
	void on_command(const string &id) override;

	void on_draw_win(MultiView::Window *win) override;

	Array<int> selection;
};

#endif /* MODEMODELMESHCREATEPOLYGON_H_ */
