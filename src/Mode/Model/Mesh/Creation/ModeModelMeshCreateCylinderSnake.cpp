/*
 * ModeModelMeshCreateCylinderSnake.cpp
 *
 *  Created on: 09.01.2013
 *      Author: michi
 */

#include "ModeModelMeshCreateCylinderSnake.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../Data/Model/Geometry/GeometryCylinder.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../lib/nix/nix.h"

const float CYLINDER_CLOSING_DISTANCE = 20;

namespace MultiView{
	float snap_f(MultiView *mv, float f);
	string format_length(MultiView *mv, float l);
}


ModeModelMeshCreateCylinderSnake::ModeModelMeshCreateCylinderSnake(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreateCylinderSnake", _parent)
{
	message = _("Zylinderschlange... Punkte + [Ctrl + Return]");

	radius = 0;
	closed = false;
	ready_for_scaling = false;
	geo = NULL;
}

void ModeModelMeshCreateCylinderSnake::on_start()
{
	dialog = hui::CreateResourceDialog("new_cylinder_dialog",ed);

	dialog->set_int("rings", hui::Config.get_int("NewCylinderRings", 4));
	dialog->set_int("edges", hui::Config.get_int("NewCylinderEdges", 8));
	dialog->check("round", hui::Config.get_bool("NewCylinderRound", false));
	dialog->hide_control("type:visible", true);
	dialog->hide_control("type:physical", true);
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->show();
	dialog->event("hui:close", std::bind(&ModeModelMeshCreateCylinderSnake::onClose, this));

	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);

	ed->activate("");
}


void ModeModelMeshCreateCylinderSnake::on_end()
{
	delete(dialog);
	if (geo)
		delete(geo);
}

void ModeModelMeshCreateCylinderSnake::updateGeometry()
{
	if (geo)
		delete(geo);
	if (ready_for_scaling){
		bool round = dialog->is_checked("round");
		int rings = dialog->get_int("rings");
		int edges = dialog->get_int("edges");
		hui::Config.set_int("NewCylinderRings", rings);
		hui::Config.set_int("NewCylinderEdges", edges);
		hui::Config.set_bool("NewCylinderRound", round);

		geo = new GeometryCylinder(pos, radius, rings * (pos.num - 1), edges, closed ? GeometryCylinder::END_LOOP : (round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT));
	}
}


void ModeModelMeshCreateCylinderSnake::on_mouse_move()
{
	if (ready_for_scaling){
		vector p = multi_view->getCursor3d(pos.back());
		radius = (p - pos.back()).length();
		if (multi_view->snap_to_grid)
			radius = MultiView::snap_f(multi_view, radius);
		float min_rad = 10 / multi_view->active_win->zoom(); // 10 px
		if (radius < min_rad)
			radius = min_rad;
		updateGeometry();
		message = _("Zylinderradius: ") + MultiView::format_length(multi_view, radius);
	}
}



void ModeModelMeshCreateCylinderSnake::on_left_button_up()
{
	if (ready_for_scaling){

		data->pasteGeometry(*geo, mode_model_mesh->current_material);
		data->selectOnlySurface(&data->surface.back());

		abort();
	}else{
		if (pos.num > 2){
			vector pp = multi_view->mouse_win->project(pos[0]);
			pp.z = 0;
			if ((pp - multi_view->m).length_fuzzy() < CYLINDER_CLOSING_DISTANCE){
				closed = true;
				ready_for_scaling = true;
				on_mouse_move();
				message = _("Zylinder: Radius");
				updateGeometry();
				ed->force_redraw();
				return;
			}

		}
		pos.add(multi_view->getCursor3d());
	}
}



void ModeModelMeshCreateCylinderSnake::on_command(const string& id)
{
	if (id == "finish-action"){
		if (pos.num > 1){
			ready_for_scaling = true;
			on_mouse_move();
			message = _("Zylinderradius: ");
			updateGeometry();
			ed->force_redraw();
		}
	}
}





void ModeModelMeshCreateCylinderSnake::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	if (pos.num > 0){

		// control points
		nix::SetColor(multi_view->ColorCreationLine);
		nix::SetShader(nix::default_shader_2d);
		for (int i=0;i<pos.num;i++){
			vector pp = win->project(pos[i]);
			nix::DrawRect(pp.x - 3, pp.x + 3, pp.y - 3, pp.y + 3, 0);
		}


		// control polygon
		nix::SetColor(ColorInterpolate(multi_view->ColorCreationLine, multi_view->ColorBackGround, 0.3f));
		MultiView::set_wide_lines(2);
		for (int i=1;i<pos.num;i++)
			nix::DrawLine3D(pos[i - 1], pos[i]);

		if ((!ready_for_scaling) and (pos.num > 0))
			nix::DrawLine3D(pos.back(), multi_view->getCursor3d());


		// spline curve
		Interpolator<vector> inter(Interpolator<vector>::TYPE_CUBIC_SPLINE_NOTANG);
		for (vector &p: pos)
			inter.add(p);
		if (!ready_for_scaling)
			inter.add(multi_view->getCursor3d());
		inter.normalize();
		nix::SetColor(multi_view->ColorCreationLine);
		for (int i=0;i<100;i++)
			nix::DrawLine3D(inter.get((float)i * 0.01f), inter.get((float)i * 0.01f + 0.01f));
	}
	if (ready_for_scaling){
		geo->build(nix::vb_temp);
		mode_model->set_material_creation();
		nix::Draw3D(nix::vb_temp);
	}else if (pos.num > 2){
		vector pp = multi_view->mouse_win->project(pos[0]);
		pp.z = 0;
		if ((pp - multi_view->m).length_fuzzy() < CYLINDER_CLOSING_DISTANCE){
			ed->draw_str(pp.x, pp.y, _("Pfad schlie&sen"));
		}
	}
}

void ModeModelMeshCreateCylinderSnake::onClose()
{
	abort();
}
