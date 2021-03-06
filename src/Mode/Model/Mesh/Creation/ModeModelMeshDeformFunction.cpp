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
	geo = nullptr;
	f = nullptr;
	has_preview = false;
	coord_system = CoordSystem::RELATIVE;

	Image im;
	im.create(512, 512, White);
	for (int i=0; i<=8; i++) {
		int x = ::min(i * 64, 511);
		for (int y=0; y<512; y++) {
			im.set_pixel(x, y, Gray);
			im.set_pixel(y, x, Gray);
		}
	}

	tex = new nix::Texture;
	tex->overwrite(im);

	//mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);
}

ModeModelMeshDeformFunction::~ModeModelMeshDeformFunction() {
	delete tex;
}

void ModeModelMeshDeformFunction::on_start() {
	msg_error("maybe we should overthink script management....");
	// Dialog
	dialog = new hui::Panel();
	dialog->from_resource("deformation_function_dialog");
	//dialog->setFont("source", "Monospace 10");
	//dialog->setTabSize("source", 4);
	dialog->set_string("source", "vector f(vector v)\n\treturn vector(v.x, v.y+(v.x*v.x-v.x), v.z)\n");
	dialog->set_int("coord", (int)coord_system);
	dialog->event("preview", [=]{ on_preview(); });
	dialog->event("ok", [=]{ on_ok(); });
	ed->set_side_panel(dialog);

	//ed->activate("");

	data->getBoundingBox(max, min);
	bool first = true;
	foreachi(ModelVertex &v, data->mesh->vertex, i)
		if (v.is_selected) {
			index.add(i);
			old_pos.add(v.pos);
			if (first) {
				min = v.pos;
				max = v.pos;
			} else {
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

void ModeModelMeshDeformFunction::on_end() {
	if (has_preview)
		restore();
	ed->set_side_panel(nullptr);
	delete geo;
	script = nullptr;
}

void ModeModelMeshDeformFunction::on_draw_win(MultiView::Window* win) {
	parent->on_draw_win(win);

	ModeModel::set_material_creation();
	geo->build(nix::vb_temp);

	nix::set_texture(tex);
	nix::draw_triangles(nix::vb_temp);
}

vector ModeModelMeshDeformFunction::transform(const vector &v) {
	vector v0 = v_0;
	vector scale = vector(1,1,1);

	if (coord_system == CoordSystem::RELATIVE) {
		v0 = min;
		scale = max - min;
	} else if (coord_system == CoordSystem::RELATIVE_SYM) {
		v0 = (min + max) / 2;
		scale = (max - min) / 2;
	}
	vector vv = v - v0;
	vv = vector(vv.x / scale.x, vv.y / scale.y, vv.z / scale.z);
	vector w = (*f)(vv);
	return vector(w.x * scale.x, w.y * scale.y, w.z * scale.z) + v0;
}

void ModeModelMeshDeformFunction::on_preview() {
	if (has_preview)
		restore();

	update_function();

	if (!f)
		return;

	for (ModelVertex &v: geo->vertex)
		v.pos = transform(v.pos);

	for (int vi: index)
		data->mesh->vertex[vi].pos = transform(data->mesh->vertex[vi].pos);
	data->notify();
	has_preview = true;
	multi_view->force_redraw();
}

void ModeModelMeshDeformFunction::update_function() {
	coord_system = (CoordSystem)dialog->get_int("coord");

	f = nullptr;
	try {
		script = kaba::create_for_source(dialog->get_string("source"));
		f = (vec_func*)script->match_function("*", "math.vector", {"math.vector"});

		if (!f)
			hui::ErrorBox(ed, "error", _("no function of type 'vector f(vector)' found"));

	} catch (kaba::Exception &e) {
		hui::ErrorBox(ed, "error", e.message());
		f = nullptr;
	}
}

void ModeModelMeshDeformFunction::restore() {
	delete geo;
	vector d = max - min;
	geo = new GeometryCube(min, vector::EX * d.x, vector::EY * d.y, vector::EZ * d.z, CUBE_SIZE, CUBE_SIZE, CUBE_SIZE);
	multi_view->force_redraw();

	foreachi(int vi, index, ii)
		data->mesh->vertex[vi].pos = old_pos[ii];
	data->notify();

	has_preview = false;
}

void ModeModelMeshDeformFunction::on_ok() {
	if (has_preview)
		restore();
	update_function();

	if (!f)
		return;

	data->begin_action_group("deformation");
	for (int vi: index)
		data->execute(new ActionModelMoveVertex(vi, transform(data->mesh->vertex[vi].pos)));

	data->end_action_group();

	abort();
}
