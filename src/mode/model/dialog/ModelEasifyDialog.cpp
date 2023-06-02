/*
 * ModelEasifyDialog.cpp
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#include "ModelEasifyDialog.h"
#include "../../../data/model/DataModel.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../Edward.h"

ModelEasifyDialog::ModelEasifyDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("easify_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("easify_dialog");

	data = _data;
	factor = 0.5f;

	event("hui:close", [=]{ on_close(); });
	event("cancel", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
	event("quality_factor", [=]{ on_quality_factor(); });
	event("quality_slider", [=]{ on_quality_slider(); });

	load_data();
}

void ModelEasifyDialog::load_data() {
	set_float("quality_factor", factor * 100.0f);
	set_float("quality_slider", factor);
	set_string("eat_vertices", format(_("Vertices: %d (of %d)"), (int)(data->mesh->vertex.num * factor), data->mesh->vertex.num));
	set_string("eat_polygons", format(_("Polygons: %d (of %d)"), (int)(data->mesh->polygon.num * factor), data->mesh->polygon.num));
}

void ModelEasifyDialog::on_quality_factor() {
	factor = get_float("quality_factor") / 100.0f;
	load_data();
}

void ModelEasifyDialog::on_quality_slider() {
	factor = get_float("");
	load_data();
}

void ModelEasifyDialog::on_close() {
	request_destroy();
}

void ModelEasifyDialog::on_ok() {
	data->easify(factor);
	request_destroy();
}


