/*
 * ModeModelMeshVertex.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHVERTEX_H_
#define MODEMODELMESHVERTEX_H_

#include "ModeModelMesh.h"

class ModeModelMeshVertex: public Mode
{
public:
	ModeModelMeshVertex(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshVertex();

	virtual void Start();
	virtual void End();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnKeyDown();
	virtual void OnKeyUp();
	virtual void OnCommand(const string &id);
	virtual void OnDataChange();

	virtual void Draw();

	DataModel *data;
};

extern ModeModelMeshVertex *mode_model_mesh_vertex;

#endif /* MODEMODELMESHVERTEX_H_ */
