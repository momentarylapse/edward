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

	virtual void onStart();
	virtual void onEnd();

	virtual void onUpdate(Observable *o, const string &message);

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
	virtual void onSelectionChange();

	// for MultiView actions
	void getSelectedSkinVertices(Array<int> &surf, Array<int> &tria, Array<int> &index);


	void fetchData();
	int getNumSelected();

	Array<ModelSkinVertexDummy> skin_vertex;

	int current_texture_level;
	void setCurrentTextureLevel(int level);
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */
