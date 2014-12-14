/*
 * ModelEasifyDialog.cpp
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#include "ModelEasifyDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelEasifyDialog::ModelEasifyDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data):
	HuiWindow("easify_dialog", _parent, _allow_parent),
	Observer("ModelEasifyDialog")
{

	data = _data;
	factor = 0.5f;

	event("hui:close", this, &ModelEasifyDialog::OnClose);
	event("cancel", this, &ModelEasifyDialog::OnClose);
	event("ok", this, &ModelEasifyDialog::OnOk);
	event("quality_factor", this, &ModelEasifyDialog::OnQualityFactor);
	event("quality_slider", this, &ModelEasifyDialog::OnQualitySlider);

	LoadData();
}

ModelEasifyDialog::~ModelEasifyDialog()
{
}

void ModelEasifyDialog::LoadData()
{
	setFloat("quality_factor", factor * 100.0f);
	setFloat("quality_slider", factor);
	setString("eat_vertices", format(_("Vertexpunkte: %d (von %d)"), (int)(data->Vertex.num * factor), data->Vertex.num));
	setString("eat_polygons", format(_("Polygone: %d (von %d)"), (int)(data->GetNumPolygons() * factor), data->GetNumPolygons()));
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
	delete(this);
}

void ModelEasifyDialog::OnOk()
{
	data->Easify(factor);
	delete(this);
}

void ModelEasifyDialog::onUpdate(Observable* o, const string &message)
{
	LoadData();
}


