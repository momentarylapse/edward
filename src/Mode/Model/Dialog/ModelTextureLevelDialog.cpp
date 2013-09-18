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

ModelTextureLevelDialog::ModelTextureLevelDialog(HuiWindow *_parent, DataModel *_data):
	EmbeddedDialog(_parent, "model_texture_level_dialog", "root-table", 1, 0, "width=250,noexpandx")
{
	data = _data;

	win->SetTooltip("texture_list", _("Texturen der einzelnen Materialien\n- Klick zum Bearbeiten der Texturkoordinaten"));

	//win->EventMX("texture_list", "hui:change", this, &ModelTextureLevelDialog::OnTextureListCheck);
	//win->EventMX("texture_list", "hui:activate", this, &ModelTextureLevelDialog::OnTextureList);
	win->EventMX("texture_list", "hui:select", this, &ModelTextureLevelDialog::OnTextureList);

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
	int n_sel = -1;
	foreachi(ModelMaterial &m, data->Material, i){
		string im = render_material(&m);
		AddString("texture_list", format("Mat[%d]\\%s\\%s", i, im.c_str(), file_secure(m.MaterialFile).c_str()));
	}
	int n = data->Material.num;
	foreachi(ModelMaterial &m, data->Material, i)
		for (int j=0;j<m.NumTextures;j++){
			string im = ed->get_tex_image(m.Texture[j]);
			win->AddChildString("texture_list", i, format("Tex[%d]\\%s\\%s", j, im.c_str(), file_secure(m.TextureFile[j]).c_str()));
			if ((i == mode_model_mesh->CurrentMaterial) && (j == mode_model_mesh_texture->CurrentTextureLevel))
				n_sel = n;
			n ++;
		}
	win->ExpandAll("texture_list", true);
	SetInt("texture_list", n_sel);
}



void ModelTextureLevelDialog::OnUpdate(Observable *o)
{
	LoadData();
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




