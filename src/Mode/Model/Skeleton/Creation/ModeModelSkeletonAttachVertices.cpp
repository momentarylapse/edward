/*
 * ModeModelSkeletonAttachVertices.cpp
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#include "ModeModelSkeletonAttachVertices.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../MultiView/DrawingHelper.h"
#include "../../../../MultiView/ColorScheme.h"
#include "../../../../Data/Model/ModelMesh.h"
#include "../../../../Stuff/BrushPanel.h"
#include "../../../../y/ResourceManager.h"
#include "../../Mesh/Selection/MeshSelectionModePolygon.h"


static void set_weight(ivec4 &bones, vec4 &weights, int bone, float weight) {
	int prev = bones.find(bone);
	if (prev >= 0) {
		// sum(other) != 1-weight
		weights *= (1-weight) / max(weights.sum() - weights[prev], 0.001f);
		weights[prev] = weight;
	} else {
		prev = weights.argmin();
		weights *= (1-weight) / max(weights.sum() - weights[prev], 0.001f);
		weights[prev] = weight;
		bones[prev] = bone;
	}

	weights /= weights.sum();
}

static float get_weight(ivec4 &bones, vec4 &weights, int bone) {
	int prev = bones.find(bone);
	if (prev >= 0)
		return weights[prev];
	return 0;
}

static void add_weight(ivec4 &bones, vec4 &weights, int bone, float dweight) {
	set_weight(bones, weights, bone, min(get_weight(bones, weights, bone) + dweight, 1.0f));
}



ModeModelSkeletonAttachVertices::ModeModelSkeletonAttachVertices(ModeBase* _parent, int _bone_index) :
	ModeCreation<DataModel>("ModelSkeletonAttachVertices", _parent)
{
	message = _("Select vertices, [Ctrl + Return] = done");
	bone_index = _bone_index;
	brushing = false;
	distance = 1;
}

void ModeModelSkeletonAttachVertices::on_start() {

	dialog = new BrushPanel(multi_view, "model-texture-paint-brush-dialog");
	ed->set_side_panel(dialog);

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_polygon);
	ed->mode_model->allow_selection_modes(false);

	multi_view->set_allow_action(false);
	multi_view->set_allow_select(false);

	if (!vb_weight)
		vb_weight = new nix::VertexBuffer("3f,3fn,2f,f");
	VertexStagingBuffer vbs;
#if 0
	for (ModelPolygon &t: data->mesh->polygon) {
		t.add_to_vertex_buffer(data->mesh->vertex, vbs, 1);
		vbs.build(vb_weight, 1);
	}
#endif
	ed->error_box("TODO....");
	if (!shader) {
		try {
			shader = ResourceManager::load_shader("vertex-weight.shader");
		} catch(Exception &e) {
			msg_error(e.message());
		}
	}

	data->subscribe(this, [=]{ on_data_change(); });

	on_data_change();
}

void ModeModelSkeletonAttachVertices::on_end() {
	data->unsubscribe(this);
	multi_view->unsubscribe(this);

	ed->set_side_panel(nullptr);

	ed->mode_model->allow_selection_modes(false);

	//parent->on_update(data, "");
}

void ModeModelSkeletonAttachVertices::on_data_change() {
	mode_model_mesh->selection_mode->update_multi_view();

	// TODO
#if 0
	Array<float> ww;
	for (ModelPolygon &t: data->mesh->polygon) {
		for (int i=0; i<t.side.num-2; i++) {
			auto &a = t.side[t.side[i].triangulation[0]];
			auto &b = t.side[t.side[i].triangulation[1]];
			auto &c = t.side[t.side[i].triangulation[2]];
			auto &va = data->mesh->vertex[a.vertex];
			auto &vb = data->mesh->vertex[b.vertex];
			auto &vc = data->mesh->vertex[c.vertex];
			ww.add(get_weight(va.bone_index, va.bone_weight, bone_index));
			ww.add(get_weight(vb.bone_index, vb.bone_weight, bone_index));
			ww.add(get_weight(vc.bone_index, vc.bone_weight, bone_index));
		}
	}
	vb_weight->update(3, ww);
#endif
}

void ModeModelSkeletonAttachVertices::on_command(const string &id) {
}


BrushPanel *ModeModelSkeletonAttachVertices::brush_panel() {
	return static_cast<BrushPanel*>(dialog);
}

void ModeModelSkeletonAttachVertices::on_draw_win(MultiView::Window *win) {
	mode_model_mesh->on_draw_win(win);

	// weights
	set_material_selected();
	nix::set_shader(shader);
	nix::draw_triangles(vb_weight);
	nix::set_offset(0);
	nix::disable_alpha();


	if (multi_view->hover.index >= 0) {
		vector pos = multi_view->hover.point;
		vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;

		set_color(scheme.CREATION_LINE);
		set_line_width(scheme.LINE_WIDTH_MEDIUM);
		float radius = brush_panel()->radius0();
		draw_circle(pos, n, radius);
	}
}

void ModeModelSkeletonAttachVertices::on_mouse_move() {
	if (!brushing)
		return;
	if (multi_view->hover.index < 0)
		return;
	vector pos = multi_view->hover.point;
	distance += (pos - last_pos).length();
	last_pos = pos;
	if (distance > brush_panel()->radius() * 0.7f) {
		distance = 0;
		apply();
	}
}

void ModeModelSkeletonAttachVertices::on_left_button_down() {
	if (multi_view->hover.index < 0)
		return;
	data->begin_action_group("brush");
	vector pos = multi_view->hover.point;
	distance = 0;
	last_pos = pos;
	brushing = true;

	apply();
}

void ModeModelSkeletonAttachVertices::on_left_button_up() {
	if (brushing)
		data->end_action_group();
	brushing = false;
}

void ModeModelSkeletonAttachVertices::on_set_multi_view() {
	mode_model_mesh->on_set_multi_view();
}

void ModeModelSkeletonAttachVertices::apply() {

	vector pos = multi_view->hover.point;
	vector n = data->mesh->polygon[multi_view->hover.index].temp_normal;

	auto bp = brush_panel();
	auto brush = bp->prepare(pos);


	Array<int> index;
	Array<ivec4> bone;
	Array<vec4> bone_weight;
	foreachi(auto &v, data->edit_mesh->vertex, i) {
		float x = brush.get(v.pos);
		if (x == 0)
			continue;
		auto bb = v.bone_index;
		auto ww = v.bone_weight;
		add_weight(bb, ww, bone_index, x);
		index.add(i);
		bone.add(bb);
		bone_weight.add(ww);
	}
	if (index.num > 0)
		data->boneAttachVertices(index, bone, bone_weight);
}
