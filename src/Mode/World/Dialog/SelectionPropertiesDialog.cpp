/*
 * SelectionPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "SelectionPropertiesDialog.h"
#include "../../../Edward.h"

SelectionPropertiesDialog::SelectionPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataWorld *_data, int *_selected_type, int *_selected_index):
	HuiWindow("world_ps_dialog", _parent, _allow_parent)
{
	data = _data;
	selected_index = _selected_index;
	selected_type = _selected_type;
	*selected_index = -1;
	*selected_type = -1;

	event("hui:close", this, &SelectionPropertiesDialog::OnClose);
	event("property_list", this, &SelectionPropertiesDialog::OnPropertyList);

	subscribe(data);

	LoadData();
}

SelectionPropertiesDialog::~SelectionPropertiesDialog()
{
	//mode_world->SelectionDialog = NULL;
	unsubscribe(data);
}

void SelectionPropertiesDialog::LoadData()
{
	FillList();
}



void SelectionPropertiesDialog::OnPropertyList()
{
	*selected_index = index[getInt("")];
	*selected_type = type[getInt("")];
	delete(this);
}



void SelectionPropertiesDialog::FillList()
{
	HuiComboBoxSeparator = ":";
	reset("property_list");
	addString("property_list", _("Welt") + ":-:" + _(" -- Komplette Welt -- "));
	index.add(0);
	type.add(FDWorld);

	foreachi(WorldObject &o, data->Objects, i)
		if (o.is_selected){
			addString("property_list", _("Objekt") + format(":%d:", i) + o.Name);
			type.add(FDModel);
			index.add(i);
		}
	foreachi(WorldTerrain &t, data->Terrains, i)
		if (t.is_selected){
			addString("property_list", _("Terrain") + format(":%d:", i) + t.FileName);
			type.add(FDTerrain);
			index.add(i);
		}
	/*for (int i=0;i<CamPoint.num;i++)
		if (CamPoint[i].IsSelected){
			PropertySelectionDialog->AddString(HMM_PROPERTY_LIST, _("KameraPunkt") + format(":%d:  ---", i));
			PropertySelectionType.add(FDCameraFlight);
			PropertySelectionIndex.add(i);
		}*/

	HuiComboBoxSeparator = "\\";
}



void SelectionPropertiesDialog::OnClose()
{
	delete(this);
}

void SelectionPropertiesDialog::onUpdate(Observable *o)
{
	LoadData();
}




