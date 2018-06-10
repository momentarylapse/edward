/*
 * ModeModelMeshPaste.h
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshPaste: public ModeCreation<DataModel>
{
public:
	ModeModelMeshPaste(ModeBase *_parent);

	void onStart() override;
	void onEnd() override;

	void onMouseMove() override;
	void onLeftButtonUp() override;

	void onDrawWin(MultiView::Window *win) override;

	void updateGeometry();

private:
	vector dpos0;
	Geometry *geo;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHPASTE_H_ */
