/*
 * ModeModelMeshTexture.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHTEXTURE_H_
#define MODEMODELMESHTEXTURE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../lib/file/file.h"

class Mode;
class DataModel;

class ModeModelMeshTexture: public Mode
{
public:
	ModeModelMeshTexture(Mode *_parent, DataModel *_data);
	virtual ~ModeModelMeshTexture();

	void Start();
	void End();

	void OnUpdate(Observable *o);

	void Draw();
	void DrawWin(int win, irect dest);

	// for MultiView actions
	void GetSelectedSkinVertices(Array<int> &surf, Array<int> &index);

	DataModel *data;
	Array<ModeModelSkinVertexDummy> skin_vertex;
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */