/*
 * ModelEasifyDialog.cpp
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#include "ModelEasifyDialog.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

ModelEasifyDialog::ModelEasifyDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 320, 450, _parent, _allow_parent, HuiWinModeControls , true)
{
	// dialog
	FromResource("easify_dialog");

	data = _data;
	factor = 0.5f;

	EventM("hui:close", this, (void(HuiEventHandler::*)())&ModelEasifyDialog::OnClose);
	EventM("cancel", this, (void(HuiEventHandler::*)())&ModelEasifyDialog::OnClose);
	EventM("ok", this, (void(HuiEventHandler::*)())&ModelEasifyDialog::OnOk);
	EventM("quality_factor", this, (void(HuiEventHandler::*)())&ModelEasifyDialog::OnQualityFactor);

	LoadData();
}

ModelEasifyDialog::~ModelEasifyDialog()
{
}

void ModelEasifyDialog::LoadData()
{
	SetFloat("quality_factor", factor * 100.0f);
	SetInt("num_vertices", data->Vertex.num);
	SetInt("num_triangles", data->GetNumPolygons());
	SetInt("num_vertices_wanted", data->Vertex.num * factor);
	SetInt("num_triangles_wanted", data->GetNumPolygons() * factor);
}

void ModelEasifyDialog::OnQualityFactor()
{
	factor = GetFloat("quality_factor") / 100.0f;
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

void ModelEasifyDialog::OnUpdate(Observable* o)
{
	LoadData();
}


