/*
 * AdministrationDialog.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "AdministrationDialog.h"

AdministrationDialog::AdministrationDialog(CHuiWindow* _parent, bool _allow_parent):
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	//data = _data;

	// dialog
	FromResource("ad_dialog");
	/*EventM("cancel", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnClose);
	EventM("set", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::ApplyData);
	EventM("ok", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnOk);
	EventM("mat_add_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnAddTextureLevel);
	EventM("mat_textures", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTextures);
	EventMX("mat_textures", "hui:select", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTexturesSelect);
	EventM("mat_delete_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnDeleteTextureLevel);
	EventM("mat_empty_texture_level", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnEmptyTextureLevel);
	EventM("transparency_mode", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnTransparencyMode);
	EventM("reflection", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnReflection);
	EventM("reflection_textures", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnReflectionTextures);
	EventM("find_effect", this, (void(HuiEventHandler::*)())&MaterialPropertiesDialog::OnFindEffect);*/

	LoadData();
	//Subscribe(data);
}

AdministrationDialog::~AdministrationDialog()
{
}

void AdministrationDialog::LoadData()
{
}

void AdministrationDialog::OnUpdate(Observable* o)
{
}


