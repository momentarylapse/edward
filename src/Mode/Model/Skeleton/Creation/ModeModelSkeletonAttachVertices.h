/*
 * ModeModelSkeletonAttachVertices.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETONATTACHVERTICES_H_
#define MODEMODELSKELETONATTACHVERTICES_H_

#include "../../../ModeCreation.h"
#include "../../../../Data/Model/DataModel.h"

class BrushPanel;

class ModeModelSkeletonAttachVertices: public ModeCreation<DataModel> {
public:
	ModeModelSkeletonAttachVertices(ModeBase *_parent, int _bone_index);

	void on_start() override;
	void on_end() override;

	void on_command(const string &id) override;

	void on_mouse_move() override;
	void on_left_button_down() override;
	void on_left_button_up() override;

	void on_draw_win(MultiView::Window *win) override;

	void on_set_multi_view() override;

	void on_data_change();

	void apply();

private:
	int bone_index;
	bool brushing;
	float distance;
	vector last_pos;

	nix::VertexBuffer *vb_weight = nullptr;
	nix::Shader *shader = nullptr;

	BrushPanel *brush_panel();
};

#endif /* MODEMODELSKELETONATTACHVERTICES_H_ */
