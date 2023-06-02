/*
 * ModeModelMeshTexture.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMESHTEXTURE_H_
#define MODEMODELMESHTEXTURE_H_

#include "../../Mode.h"
#include "../../../data/model/DataModel.h"

class DataModel;
class ModelMaterialDialog;
class ModelSkinVertexDummy;

class ModeModelMeshTexture: public Mode<DataModel> {
public:
	ModeModelMeshTexture(ModeBase *parent, MultiView::MultiView *mv);

	class State : public Observable<VirtualBase> {
	public:
		static const string MESSAGE_TEXTURE_LEVEL_CHANGE;
		~State(){}
	} state;

	void on_start() override;
	void on_end() override;

	void on_data_skin_change();
	void on_data_change();

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
	void set_current_texture_level(int level);
};

extern ModeModelMeshTexture *mode_model_mesh_texture;

#endif /* MODEMODELMESHTEXTURE_H_ */
