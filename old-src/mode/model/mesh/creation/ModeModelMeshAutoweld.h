/*
 * ModeModelMeshAutoweld.h
 *
 *  Created on: 11.11.2013
 *      Author: michi
 */

#ifndef MODEMODELMESHAUTOWELD_H_
#define MODEMODELMESHAUTOWELD_H_

#include "../../../ModeCreation.h"
#include "../../../../data/model/DataModel.h"

class ModeModelMesh;

class ModeModelMeshAutoweld: public ModeCreation<ModeModelMesh, DataModel> {
public:
	ModeModelMeshAutoweld(ModeModelMesh *_parent);
	virtual ~ModeModelMeshAutoweld();

	void on_start() override;
	void on_end() override;

	void on_draw_win(MultiView::Window *win) override;

	void onSlider();
	void onCancel();
	void onOk();

private:
	float radius;
	float radius_default;
};

#endif /* MODEMODELMESHAUTOWELD_H_ */
