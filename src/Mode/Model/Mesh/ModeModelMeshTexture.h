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
class ModelMaterialDialog;

class ModeModelMeshTexture: public Mode<DataModel>, public Observable
{
public:
	ModeModelMeshTexture(ModeBase *parent);

	static const string MESSAGE_TEXTURE_LEVEL_CHANGE;

	void on_start() override;
	void on_end() override;

	void on_update(Observable *o, const string &message) override;

	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;
	void on_selection_change() override;

	// for MultiView actions
	void getSelectedSkinVertices(Array<int> &tria, Array<int> &index);


	void fetchData();
	int getNumSelected();

	ModelMaterialDialog *dialog;

	Array<ModelSkinVertexDummy> skin_vertex;

	int current_texture_level;
	void setCurrentTextureLevel(int level);
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */
