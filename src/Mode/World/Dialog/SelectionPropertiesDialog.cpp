/*
 * SelectionPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "SelectionPropertiesDialog.h"
#include "../../../Storage/Storage.h"

SelectionPropertiesDialog::SelectionPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data, int *_selected_type, int *_selected_index):
	hui::Dialog("world_ps_dialog", 400, 400, _parent, _allow_parent),
	Observer("SelectionPropertiesDialog")
{
	from_resource("world_ps_dialog");
	data = _data;
	selected_index = _selected_index;
	selected_type = _selected_type;
	*selected_index = -1;
	*selected_type = -1;

	event("hui:close", std::bind(&SelectionPropertiesDialog::OnClose, this));
	event("property_list", std::bind(&SelectionPropertiesDialog::OnPropertyList, this));

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
	*selected_index = index[get_int("")];
	*selected_type = type[get_int("")];
	destroy();
}



void SelectionPropertiesDialog::FillList()
{
	hui::ComboBoxSeparator = ":";
	reset("property_list");
	add_string("property_list", _("World") + ":-:" + _(" -- Whole world -- "));
	index.add(0);
	type.add(FD_WORLD);

	foreachi(WorldObject &o, data->Objects, i)
		if (o.is_selected){
			add_string("property_list", _("Object") + format(":%d:", i) + o.name);
			type.add(FD_MODEL);
			index.add(i);
		}
	foreachi(WorldTerrain &t, data->Terrains, i)
		if (t.is_selected){
			add_string("property_list", _("Terrain") + format(":%d:", i) + t.filename);
			type.add(FD_TERRAIN);
			index.add(i);
		}
	/*for (int i=0;i<CamPoint.num;i++)
		if (CamPoint[i].IsSelected){
			PropertySelectionDialog->AddString(HMM_PROPERTY_LIST, _("Camera point") + format(":%d:  ---", i));
			PropertySelectionType.add(FDCameraFlight);
			PropertySelectionIndex.add(i);
		}*/

	hui::ComboBoxSeparator = "\\";
}



void SelectionPropertiesDialog::OnClose()
{
	destroy();
}

void SelectionPropertiesDialog::on_update(Observable *o, const string &message)
{
	LoadData();
}




