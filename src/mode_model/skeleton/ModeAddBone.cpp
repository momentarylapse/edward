//
// Created by michi on 7/27/26.
//

#include "ModeAddBone.h"
#include "ModeSkeleton.h"
#include "../mesh/ModeMesh.h"
#include "../ModeModel.h"
#include "../data/ModelMesh.h"
#include <Session.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <view/DocumentSession.h>
#include <view/EdwardWindow.h>
#include <view/multiview/MultiView.h>

ModeAddBone::ModeAddBone(ModeSkeleton* parent) :
	SubMode(parent)
{
	mode_skeleton = parent;
	mode_mesh = mode_skeleton->parent->mode_mesh.get();
	multi_view = mode_skeleton->multi_view;
	generic_data = mode_skeleton->generic_data;
}

void ModeAddBone::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeAddBone::on_draw_post(Painter* p) {
	_parent->on_draw_post(p);

	draw_info(p, "click to add bone TODO");
}


void ModeAddBone::on_key_down(int key) {
	if (key == xhui::KEY_ESCAPE) {
		request_mode_end();
	}
}

void ModeAddBone::on_left_button_down(const vec2& m) {
	if (multi_view->hover and multi_view->hover->type == MultiViewType::SKELETON_BONE)
		return;
	const vec3 p = multi_view->cursor_pos_3d(m);
	mode_mesh->data->add_bone(p, -1);
	session->win->request_redraw();
}
