/*
 * ModelEasifyDialog.cpp
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#include "ModelEasifyDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelEasifyDialog::ModelEasifyDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("easify_dialog", 400, 300, _parent, _allow_parent),
	Observer("ModelEasifyDialog")
{
	fromResource("easify_dialog");

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
	setFloat("quality_factor", factor * 100.0f);
	setFloat("quality_slider", factor);
	setString("eat_vertices", format(_("Vertexpunkte: %d (von %d)"), (int)(data->vertex.num * factor), data->vertex.num));
	setString("eat_polygons", format(_("Polygone: %d (von %d)"), (int)(data->getNumPolygons() * factor), data->getNumPolygons()));
}

void ModelEasifyDialog::OnQualityFactor()
{
	factor = getFloat("quality_factor") / 100.0f;
	LoadData();
}

void ModelEasifyDialog::OnQualitySlider()
{
	factor = getFloat("");
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

void ModelEasifyDialog::onUpdate(Observable* o, const string &message)
{
	LoadData();
}


