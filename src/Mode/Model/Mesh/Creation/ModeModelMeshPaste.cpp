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

ModeModelMeshPaste::ModeModelMeshPaste(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshPaste", _parent)
{
	message = _("verschieben, [linke Maustaste] zum Einf&ugen");
	geo = NULL;

	vector min, max;
	mode_model_mesh->temp_geo.getBoundingBox(min, max);
	dpos0 = (max + min) / 2;
}

void ModeModelMeshPaste::onStart()
{
	if (mode_model_mesh->temp_geo.vertex.num > 0){
		updateGeometry();
		multi_view->setAllowSelect(false);
		multi_view->setAllowAction(false);
	}else{
		ed->setMessage(_("nichts zum Einf&ugen"));
		abort();
	}
}

void ModeModelMeshPaste::onEnd()
{
	if (geo)
		delete(geo);
}

void ModeModelMeshPaste::onMouseMove()
{
	updateGeometry();
}

void ModeModelMeshPaste::onLeftButtonUp()
{
	data->pasteGeometry(*geo, mode_model_mesh->current_material);
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), geo->vertex.num, geo->polygon.num));
	abort();
}

void ModeModelMeshPaste::onDrawWin(MultiView::Window* win)
{
	parent->onDrawWin(win);

	mode_model->setMaterialCreation();

	geo->preview(nix::vb_temp);
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
