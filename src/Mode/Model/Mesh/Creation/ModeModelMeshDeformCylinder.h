/*
 * ModeModelMeshDeformCylinder.h
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;

class ModeModelMeshDeformCylinder: public ModeCreation<DataModel>
{
public:
	ModeModelMeshDeformCylinder(ModeBase *parent);
	virtual ~ModeModelMeshDeformCylinder();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();
	virtual void onLeftButtonUp();

	virtual void onDrawWin(MultiView::Window *win);

	void onPreview();
	void onOk();
	void onClose();

	void updateHover();
	void updateParams();

//private:
	Geometry *geo;

	vector transform(const vector &v);
	void preview();
	void restore();

	vector dir;
	vector axis[2];
	float radius;
	Array<vector> param;
	int hover;

	Interpolator<float> *inter;

	bool has_preview;
	Array<int> index;
	Array<vector> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORMCYLINDER_H_ */
