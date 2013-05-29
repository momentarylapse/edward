/*
 * ModelTextureLevelDialog.cpp
 *
 *  Created on: 22.06.2012
 *      Author: michi
 */

#include "ModelTextureLevelDialog.h"
#include "../Mesh/ModeModelMesh.h"
#include "../Mesh/ModeModelMeshTexture.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Edward.h"

string file_secure(const string &filename);
string render_material(ModelMaterial *m);

ModelTextureLevelDialog::ModelTextureLevelDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data):
	CHuiWindow("dummy", -1, -1, 230, 400, _parent, _allow_parent, HuiWinModeControls | HuiWinModeResizable, true)
{
	data = _data;

	// dialog
	FromResource("model_texture_level_dialog");
	SetPositionSpecial(_parent, HuiRight | HuiTop);

	SetTooltip("texture_list", _("Texturen der einzelnen Materialien\n- Doppelklick zum Bearbeiten der Texturkoordinaten"));

	EventM("hui:close", this, &ModelTextureLevelDialog::OnClose);
	EventMX("texture_list", "hui:change", this, &ModelTextureLevelDialog::OnTextureListCheck);
	EventMX("texture_list", "hui:activate", this, &ModelTextureLevelDialog::OnTextureList);

	Subscribe(data);

	LoadData();
}

ModelTextureLevelDialog::~ModelTextureLevelDialog()
{
	Unsubscribe(data);
}

void ModelTextureLevelDialog::ApplyData()
{
}



void ModelTextureLevelDialog::FillTextureList()
{
	Reset("texture_list");
	foreachi(ModelMaterial &m, data->Material, i){
		string im = render_material(&m);
		AddString("texture_list", format("%d\\%s\\%s\\%s", i, (i == mode_model_mesh->CurrentMaterial) ? "true" : "false", im.c_str(), file_secure(m.MaterialFile).c_str()));
	}
	foreachi(ModelMaterial &m, data->Material, i)
		for (int j=0;j<m.NumTextures;j++){
			string im = ed->get_tex_image(m.Texture[j]);
			AddChildString("texture_list", i, format("%d\\%s\\%s\\%s", j, ((i == mode_model_mesh->CurrentMaterial) && (j == mode_model_mesh_texture->CurrentTextureLevel)) ? "true" : "false", im.c_str(), file_secure(m.TextureFile[j]).c_str()));
		}
	ExpandAll("texture_list", true);
}



void ModelTextureLevelDialog::OnUpdate(Observable *o)
{
	LoadData();
}



void ModelTextureLevelDialog::OnClose()
{
}



void ModelTextureLevelDialog::LoadData()
{
	FillTextureList();
}

void ModelTextureLevelDialog::OnTextureList()
{
	int n = GetInt("");
	if (n < 0)
		return;
	if (n < data->Material.num){
		mode_model_mesh->CurrentMaterial = n;
		mode_model_mesh_texture->CurrentTextureLevel = 0;
		data->Notify("Change");
	}else{
		n -= data->Material.num;
		foreachi(ModelMaterial &m, data->Material, i)
			if (n < m.NumTextures){
				mode_model_mesh->CurrentMaterial = i;
				mode_model_mesh_texture->CurrentTextureLevel = n;
				data->Notify("Change");
				return;
			}else
				n -= m.NumTextures;
	}
}



void ModelTextureLevelDialog::OnTextureListCheck()
{
}




