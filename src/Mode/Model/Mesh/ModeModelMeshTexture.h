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

class DataModel;

class ModeModelMeshTexture: public Mode<DataModel>, public Observable
{
public:
	ModeModelMeshTexture(ModeBase *parent);
	virtual ~ModeModelMeshTexture();

	virtual void onStart();
	virtual void onEnd();

	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);

	// for MultiView actions
	void GetSelectedSkinVertices(Array<int> &surf, Array<int> &tria, Array<int> &index);


	void FetchData();
	int GetNumSelected();

	Array<ModelSkinVertexDummy> skin_vertex;

	int CurrentTextureLevel;
	void SetCurrentTextureLevel(int level);
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */
