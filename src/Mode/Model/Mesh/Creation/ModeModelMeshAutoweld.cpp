/*
 * ModeModelMeshAutoweld.cpp
 *
 *  Created on: 11.11.2013
 *      Author: michi
 */

#include "ModeModelMeshAutoweld.h"
#include "../../../../Edward.h"
#include "../../../../MultiView/MultiViewWindow.h"

ModeModelMeshAutoweld::ModeModelMeshAutoweld(ModeBase* _parent) :
	ModeCreation<DataModel>("ModelMeshAutoweld", _parent)
{
}

ModeModelMeshAutoweld::~ModeModelMeshAutoweld()
{
}

void ModeModelMeshAutoweld::OnStart()
{
	radius_default = multi_view->cam.radius * 0.01f;
	radius = radius_default;

	dialog = new HuiDialog("Autoweld", 240, 60, ed, true);
	dialog->AddControlTable("", 0, 0, 1, 2, "table0");
	dialog->SetTarget("table0", 0);
	dialog->AddControlTable("", 0, 0, 3, 2, "table1");
	dialog->AddControlTable("!buttonbar", 0, 1, 2, 1, "table2");
	dialog->SetTarget("table1", 0);
	dialog->AddText("Radius", 0, 0, 0, 0, "");
	dialog->AddSlider("!expandx", 1, 0, 0, 0, "slider");
	dialog->AddText("", 2, 0, 0, 0, "radius");
	dialog->AddText("Verbindungen", 0, 1, 0, 0, "");
	dialog->AddText("", 1, 1, 0, 0, "num_connections");
	dialog->SetTarget("table2", 0);
	dialog->AddButton("Abbrechen", 0, 0, 0, 0, "cancel");
	dialog->AddButton("Ok", 1, 0, 0, 0, "ok");
	dialog->Show();

	dialog->SetFloat("slider", 0.5f);
	dialog->SetString("radius", f2s(radius, 2));

	dialog->EventM("slider", this, &ModeModelMeshAutoweld::OnSlider);
	dialog->EventM("ok", this, &ModeModelMeshAutoweld::OnOk);
	dialog->EventM("cancel", this, &ModeModelMeshAutoweld::OnCancel);
}

void ModeModelMeshAutoweld::OnEnd()
{
	delete(dialog);
}

void ModeModelMeshAutoweld::OnSlider()
{
	radius = radius_default * exp((dialog->GetFloat("slider") - 0.5f) * 5);
	dialog->SetString("radius", f2s(radius, 2));
	ed->ForceRedraw();
}

void ModeModelMeshAutoweld::OnCancel()
{
	Abort();
}

void ModeModelMeshAutoweld::OnOk()
{
	data->AutoWeldSelectedSurfaces(radius);
	Abort();
}

void ModeModelMeshAutoweld::OnDrawWin(MultiViewWindow* win)
{
	NixEnableLighting(false);
	NixSetTexture(NULL);
	NixSetColor(Green);
	float r = 5;
	int n = 0;
	for (int i=0;i<data->Surface.num;i++){
		ModelSurface *a = &data->Surface[i];
		if (!a->is_selected)
			continue;
		for (int j=i+1;j<data->Surface.num;j++){
			ModelSurface *b = &data->Surface[j];
			if (!b->is_selected)
				continue;
			foreach(int va, a->Vertex)
				foreach(int vb, b->Vertex)
					if ((data->Vertex[va].pos - data->Vertex[vb].pos).length() <= radius){
						n ++;
						vector p = win->Project(data->Vertex[va].pos);

						if ((p.z < 0) || (p.z >= 1))
							continue;
						NixDrawRect(	p.x-r,
										p.x+r,
										p.y-r,
										p.y+r,
										p.z);
					}
		}
	}
	dialog->SetString("num_connections", i2s(n));
}
