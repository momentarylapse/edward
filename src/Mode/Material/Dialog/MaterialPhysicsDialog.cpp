/*
 * MaterialPhysicsDialog.cpp
 *
 *  Created on: 25.09.2012
 *      Author: michi
 */

#include "MaterialPhysicsDialog.h"
#include "../../../Edward.h"
#include "../ModeMaterial.h"
#include "../../../Action/Material/ActionMaterialEditPhysics.h"

MaterialPhysicsDialog::MaterialPhysicsDialog(CHuiWindow* _parent, bool _allow_parent, DataMaterial* _data) :
	CHuiWindow("dummy", -1, -1, 800, 600, _parent, _allow_parent, HuiWinModeControls, true)
{
	data = _data;

	// dialog
	FromResource("material_physics_dialog");
	EventM("cancel", this, (void(HuiEventHandler::*)())&MaterialPhysicsDialog::OnClose);
	EventM("hui:close", this, (void(HuiEventHandler::*)())&MaterialPhysicsDialog::OnClose);
	EventM("ok", this, (void(HuiEventHandler::*)())&MaterialPhysicsDialog::OnOk);

	temp = data->Physics;
	LoadData();
}

MaterialPhysicsDialog::~MaterialPhysicsDialog()
{
}

void MaterialPhysicsDialog::LoadData()
{
    SetFloat("rcjump", temp.RCJump);
    SetFloat("rcstatic", temp.RCStatic);
    SetFloat("rcsliding", temp.RCSliding);
    SetFloat("rcrolling", temp.RCRolling);
    SetFloat("rvjump", temp.RCVJumpMin);
    SetFloat("rvstatic", temp.RCVSlidingMin);
}

void MaterialPhysicsDialog::ApplyData()
{
	temp.RCJump = GetFloat("rcjump");
	temp.RCStatic = GetFloat("rcstatic");
	temp.RCSliding = GetFloat("rcsliding");
	temp.RCRolling = GetFloat("rcrolling");
	temp.RCVJumpMin = GetFloat("rvjump");
	temp.RCVSlidingMin = GetFloat("rvstatic");

	data->Execute(new ActionMaterialEditPhysics(temp));
}

void MaterialPhysicsDialog::OnOk()
{
	ApplyData();
	delete(this);
}

void MaterialPhysicsDialog::OnClose()
{
	delete(this);
}


