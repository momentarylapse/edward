/*
 * ModeModelMeshDeform.h
 *
 *  Created on: 15.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORM_H_
#define SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORM_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class Geometry;
namespace Script{
	class Script;
}

class ModeModelMeshDeform: public ModeCreation<DataModel>, public HuiEventHandler
{
public:
	ModeModelMeshDeform(ModeBase *parent);
	virtual ~ModeModelMeshDeform();

	virtual void onStart();
	virtual void onEnd();

	virtual void onMouseMove();
	virtual void onLeftButtonDown();

	virtual void onDrawWin(MultiView::Window *win);

	void onPreview();
	void onOk();

private:
	vector min, max;
	Geometry *geo;
	Script::Script *s;
	typedef _cdecl void vec_func(vector&, const vector &);
	vec_func *f;

	void updateFunction();
	vector transform(const vector &v);
	void apply();
	void preview();
	void restore();

	bool has_preview;
	Array<int> index;
	Array<vector> old_pos;
};

#endif /* SRC_MODE_MODEL_MESH_CREATION_MODEMODELMESHDEFORM_H_ */
