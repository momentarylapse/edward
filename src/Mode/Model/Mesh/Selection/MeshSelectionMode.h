/*
 * MeshSelectionMode.h
 *
 *  Created on: 18.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_
#define SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_

#include "../ModeModelMesh.h"

class MeshSelectionMode {
public:
	MeshSelectionMode(ModeModelMesh *_parent);
	virtual ~MeshSelectionMode() {}

	virtual void on_start() {}
	virtual void on_end() {}

	virtual void update_selection() {}
	virtual void update_multi_view() {}

	virtual void on_draw_win(MultiView::Window *win) {}

	bool is_active();

	ModeModelMesh *parent;
	DataModel *data;
	MultiView::MultiView *multi_view;
};

#endif /* SRC_MODE_MODEL_MESH_MESHSELECTIONMODE_H_ */
