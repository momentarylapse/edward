/*
 * SelectionPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "SelectionPropertiesDialog.h"
#include "../../../Edward.h"

SelectionPropertiesDialog::SelectionPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data, int *_selected_type, int *_selected_index):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	data = _data;
	selected_index = _selected_index;
	selected_type = _selected_type;
	*selected_index = -1;
	*selected_type = -1;

	// dialog
	FromResource("world_ps_dialog");

	EventM("hui:close", this, (void(HuiEventHandler::*)())&SelectionPropertiesDialog::OnClose);
	EventM("property_list", this, (void(HuiEventHandler::*)())&SelectionPropertiesDialog::OnPropertyList);

	Subscribe(data);

	LoadData();
}

SelectionPropertiesDialog::~SelectionPropertiesDialog()
{
	//mode_world->SelectionDialog = NULL;
	Unsubscribe(data);
}

void SelectionPropertiesDialog::LoadData()
{
	FillList();
}



void SelectionPropertiesDialog::OnPropertyList()
{
	*selected_index = index[GetInt("")];
	*selected_type = type[GetInt("")];
	delete(this);
}



void SelectionPropertiesDialog::FillList()
{
	HuiComboBoxSeparator = ":";
	Reset("property_list");
	AddString("property_list", _("Welt") + ":-:" + _(" -- Komplette Welt -- "));
	index.add(0);
	type.add(FDWorld);

	foreachi(WorldObject &o, data->Objects, i)
		if (o.is_selected){
			AddString("property_list", _("Objekt") + format(":%d:", i) + o.Name);
			type.add(FDModel);
			index.add(i);
		}
	foreachi(WorldTerrain &t, data->Terrains, i)
		if (t.is_selected){
			AddString("property_list", _("Terrain") + format(":%d:", i) + t.FileName);
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

void SelectionPropertiesDialog::OnUpdate(Observable *o)
{
	LoadData();
}




