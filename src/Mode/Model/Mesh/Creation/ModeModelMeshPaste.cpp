/*
 * ModeModelMeshPaste.cpp
 *
 *  Created on: 22.12.2014
 *      Author: michi
 */

#include "ModeModelMeshPaste.h"
#include "../ModeModelMesh.h"
#include "../../ModeModel.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshPaste::ModeModelMeshPaste(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshPaste", _parent)
{
	message = _("verschieben, [linke Maustaste] zum Einf&ugen");
	geo = NULL;

	vector min, max;
	mode_model_mesh->temp_geo.getBoundingBox(min, max);
	dpos0 = (max + min) / 2;
}

void ModeModelMeshPaste::on_start()
{
	if (mode_model_mesh->temp_geo.vertex.num == 0){
		ed->set_message(_("nichts zum Einf&ugen"));
		abort();
		return;
	}

	updateGeometry();
	multi_view->setAllowSelect(false);
	multi_view->setAllowAction(false);
}

void ModeModelMeshPaste::on_end()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshPaste::on_mouse_move()
{
	updateGeometry();
}

void ModeModelMeshPaste::on_left_button_up()
{
	data->pasteGeometry(*geo, mode_model_mesh->current_material);
	ed->set_message(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), geo->vertex.num, geo->polygon.num));
	abort();
}

void ModeModelMeshPaste::on_draw_win(MultiView::Window* win)
{
	parent->on_draw_win(win);

	mode_model->setMaterialCreation();

	geo->build(nix::vb_temp);
	nix::Draw3D(nix::vb_temp);
}

void ModeModelMeshPaste::updateGeometry()
{
	if (geo)
		delete(geo);
	matrix m;
	MatrixTranslation(m, multi_view->getCursor3d() - dpos0);
	geo = new Geometry;
	geo->add(mode_model_mesh->temp_geo);
	geo->transform(m);
}
