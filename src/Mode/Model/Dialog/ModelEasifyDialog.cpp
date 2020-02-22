/*
 * ModelEasifyDialog.cpp
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#include "ModelEasifyDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Data/Model/ModelMesh.h"
#include "../../../Edward.h"

ModelEasifyDialog::ModelEasifyDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("easify_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("easify_dialog");

	data = _data;
	factor = 0.5f;

	event("hui:close", std::bind(&ModelEasifyDialog::OnClose, this));
	event("cancel", std::bind(&ModelEasifyDialog::OnClose, this));
	event("ok", std::bind(&ModelEasifyDialog::OnOk, this));
	event("quality_factor", std::bind(&ModelEasifyDialog::OnQualityFactor, this));
	event("quality_slider", std::bind(&ModelEasifyDialog::OnQualitySlider, this));

	LoadData();
}

ModelEasifyDialog::~ModelEasifyDialog()
{
}

void ModelEasifyDialog::LoadData()
{
	set_float("quality_factor", factor * 100.0f);
	set_float("quality_slider", factor);
	set_string("eat_vertices", format(_("Vertices: %d (of %d)"), (int)(data->mesh->vertex.num * factor), data->mesh->vertex.num));
	set_string("eat_polygons", format(_("Polygons: %d (of %d)"), (int)(data->mesh->polygon.num * factor), data->mesh->polygon.num));
}

void ModelEasifyDialog::OnQualityFactor()
{
	factor = get_float("quality_factor") / 100.0f;
	LoadData();
}

void ModelEasifyDialog::OnQualitySlider()
{
	factor = get_float("");
	LoadData();
}

void ModelEasifyDialog::OnClose()
{
	destroy();
}

void ModelEasifyDialog::OnOk()
{
	data->easify(factor);
	destroy();
}


