/*
 * ModeModelMeshDeform.cpp
 *
 *  Created on: 15.12.2014
 *      Author: michi
 */


#include "../../ModeModel.h"
#include "../ModeModelMesh.h"
#include "../../../../Data/Model/Geometry/GeometryCube.h"
#include "../../../../Edward.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../lib/kaba/kaba.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"
#include "../../../../Action/ActionGroup.h"
#include "../../../../Action/Model/Mesh/Vertex/Helper/ActionModelMoveVertex.h"
#include "../Selection/MeshSelectionModePolygon.h"
#include "ModeModelMeshDeformFunction.h"

const int CUBE_SIZE = 20;

 ModeModelMeshDeformFunction::ModeModelMeshDeformFunction(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshDeformFunction", _parent)
{
	geo = NULL;
	f = NULL;
	s = NULL;
	has_preview = false;

	Image im;
	im.create(512, 512, White);
	for (int i=0; i<=8; i++){
		int x = ::min(i * 64, 511);
		for (int y=0; y<512; y++){
			im.set_pixel(x, y, Gray);
			im.set_pixel(y, x, Gray);
		}
	}

	tex = new nix::Texture;
	tex->overwrite(im);

	//mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);
}

ModeModelMeshDeformFunction::~ModeModelMeshDeformFunction()
{
	delete(tex);
}

void ModeModelMeshDeformFunction::on_start()
{
	// Dialog
	dialog = hui::CreateResourceDialog("deformation_function_dialog", ed);
	//dialog->setFont("source", "Monospace 10");
	//dialog->setTabSize("source", 4);
	dialog->set_string("source", "void f(vector o, vector i)\n\to = vector(i.x, i.y+(i.x*i.x-i.x), i.z)\n");
	dialog->set_position_special(ed, hui::HUI_RIGHT | hui::HUI_TOP);
	dialog->event("hui:close", std::bind(&ModeModelMeshDeformFunction::onClose, this));
	dialog->event("preview", std::bind(&ModeModelMeshDeformFunction::onPreview, this));
	dialog->event("ok", std::bind(&ModeModelMeshDeformFunction::onOk, this));
	dialog->show();

	//ed->activate("");

	data->getBoundingBox(max, min);
	bool first = true;
	foreachi(ModelVertex &v, data->vertex, i)
		if (v.is_selected){
			index.add(i);
			old_pos.add(v.pos);
			if (first){
				min = v.pos;
				max = v.pos;
			}else{
				min._min(v.pos);
				max._max(v.pos);
			}
			first = false;
		}

	vector d = max - min;
	geo = new GeometryCube(min, vector::EX * d.x, vector::EY * d.y, vector::EZ * d.z, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}

void ModeModelMeshDeformFunction::on_end()
{
	if (has_preview)
		restore();
	delete(dialog);
	delete(geo);
	if (s)
		delete s;
}

void ModeModelMeshDeformFunction::on_draw_win(MultiView::Window* win)
{
	parent->on_draw_win(win);

	ModeModel::set_material_creation();
	geo->build(nix::vb_temp);

	nix::SetTexture(tex);
	nix::Draw3D(nix::vb_temp);
}

vector ModeModelMeshDeformFunction::transform(const vector &v)
{
	vector d = max - min;
	vector vv = v - min;
	vv = vector(vv.x / d.x, vv.y / d.y, vv.z / d.z);
	vector w;
	(*f)(w, vv);
	return min + vector(w.x * d.x, w.y * d.y, w.z * d.z);
}

void ModeModelMeshDeformFunction::onPreview()
{
	if (has_preview)
		restore();

	updateFunction();

	if (!f)
		return;

	for (ModelVertex &v: geo->vertex)
		v.pos = transform(v.pos);

	for (int vi: index)
			data->vertex[vi].pos = transform(data->vertex[vi].pos);
	data->notify();
	has_preview = true;
	ed->force_redraw();
}

void ModeModelMeshDeformFunction::updateFunction()
{
	if (s)
		delete s;
	Kaba::Script *s = NULL;
	f = NULL;
	try{
		s = Kaba::CreateForSource(dialog->get_string("source"));
		f = (vec_func*)s->match_function("*", "void", {"vector", "vector"});

		if (!f)
			hui::ErrorBox(dialog, "error", _("keine Funktion vom Typ 'void f(vector, vector)' gefunden"));

	}catch(Kaba::Exception &e){
		hui::ErrorBox(dialog, "error", e.message());
		f = NULL;
	}
}

void ModeModelMeshDeformFunction::restore()
{
	delete(geo);
	vector d = max - min;
	geo = new GeometryCube(min, vector::EX * d.x, vector::EY * d.y, vector::EZ * d.z, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
	ed->force_redraw();

	foreachi(int vi, index, ii)
		data->vertex[vi].pos = old_pos[ii];
	data->notify();

	has_preview = false;
}

void ModeModelMeshDeformFunction::onOk()
{
	if (has_preview)
		restore();
	updateFunction();

	if (!f)
		return;

	data->begin_action_group("deformation");
	for (int vi: index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->vertex[vi].pos)));

	data->end_action_group();

	abort();
}

void ModeModelMeshDeformFunction::onClose()
{
	abort();
}
