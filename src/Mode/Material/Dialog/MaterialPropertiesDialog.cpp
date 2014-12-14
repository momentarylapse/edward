/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../../../Edward.h"
#include "../ModeMaterial.h"
#include "../../../Action/Material/ActionMaterialEditAppearance.h"
#include "../../../Action/Material/ActionMaterialEditPhysics.h"
#include "../../../lib/nix/nix.h"


string file_secure(const string &filename); // -> ModelPropertiesDialog

extern string NixShaderError; // -> nix

MaterialPropertiesDialog::MaterialPropertiesDialog(HuiWindow *_parent, DataMaterial *_data):
	EmbeddedDialog(_parent, "material_dialog", "root-table", 1, 0, "noexpandx"),
	Observer("MaterialPropertiesDialog")
{
	data = _data;

	// dialog
	win->event("mat_add_texture_level", this, &MaterialPropertiesDialog::OnAddTextureLevel);
	win->event("mat_textures", this, &MaterialPropertiesDialog::OnTextures);
	win->eventX("mat_textures", "hui:select", this, &MaterialPropertiesDialog::OnTexturesSelect);
	win->event("mat_delete_texture_level", this, &MaterialPropertiesDialog::OnDeleteTextureLevel);
	win->event("mat_empty_texture_level", this, &MaterialPropertiesDialog::OnEmptyTextureLevel);
	win->event("transparency_mode:none", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->event("transparency_mode:function", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->event("transparency_mode:color_key", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->event("transparency_mode:factor", this, &MaterialPropertiesDialog::OnTransparencyMode);
	win->event("reflection_mode:none", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->event("reflection_mode:cube_static", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->event("reflection_mode:cube_dynamic", this, &MaterialPropertiesDialog::OnReflectionMode);
	win->event("reflection_textures", this, &MaterialPropertiesDialog::OnReflectionTextures);
	win->event("find_shader", this, &MaterialPropertiesDialog::OnFindShader);


	win->event("mat_am", this, &MaterialPropertiesDialog::ApplyData);
	win->event("mat_di", this, &MaterialPropertiesDialog::ApplyData);
	win->event("mat_sp", this, &MaterialPropertiesDialog::ApplyData);
	win->event("mat_em", this, &MaterialPropertiesDialog::ApplyData);
	win->event("mat_shininess", this, &MaterialPropertiesDialog::ApplyDataDelayed);

	win->event("alpha_factor", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->event("alpha_source", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->event("alpha_dest", this, &MaterialPropertiesDialog::ApplyDataDelayed);
	win->event("alpha_z_buffer", this, &MaterialPropertiesDialog::ApplyData);

	win->event("rcjump", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->event("rcstatic", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->event("rcsliding", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);
	win->event("rcroll", this, &MaterialPropertiesDialog::ApplyPhysDataDelayed);

	win->expand("material_dialog_grp_color", 0, true);

	temp = data->Appearance;
	temp_phys = data->Physics;
	apply_queue_depth = 0;
	apply_phys_queue_depth = 0;
	LoadData();
	subscribe(data);
}

void MaterialPropertiesDialog::LoadData()
{
	FillTextureList();
	win->setColor("mat_am", temp.ColorAmbient);
	win->setColor("mat_di", temp.ColorDiffuse);
	win->setColor("mat_sp", temp.ColorSpecular);
	win->setColor("mat_em", temp.ColorEmissive);
	setFloat("mat_shininess", temp.ColorShininess);

	if (temp.TransparencyMode == TransparencyModeColorKeySmooth)
		check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeColorKeyHard)
		check("transparency_mode:color_key", true);
	else if (temp.TransparencyMode == TransparencyModeFactor)
		check("transparency_mode:factor", true);
	else if (temp.TransparencyMode == TransparencyModeFunctions)
		check("transparency_mode:function", true);
	else
		check("transparency_mode:none", true);
	enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	setFloat("alpha_factor", temp.AlphaFactor * 100.0f);
	check("alpha_z_buffer", temp.AlphaZBuffer);
	setInt("alpha_source", temp.AlphaSource);
	setInt("alpha_dest", temp.AlphaDestination);

	if (temp.ReflectionMode == ReflectionCubeMapStatic)
		check("reflection_mode:cube_static", true);
	else if (temp.ReflectionMode == ReflectionCubeMapDynamical)
		check("reflection_mode:cube_dynamic", true);
	else
		check("reflection_mode:none", true);
	setInt("reflection_size", temp.ReflectionSize);
	setInt("reflection_density", temp.ReflectionDensity);
	RefillReflTexView();
	enable("reflection_size", ((temp.ReflectionMode == ReflectionCubeMapStatic) || (temp.ReflectionMode == ReflectionCubeMapDynamical)));
	enable("reflection_textures", (temp.ReflectionMode == ReflectionCubeMapStatic));
	enable("reflection_density", (temp.ReflectionMode != ReflectionNone));
	setString("shader_file", temp.ShaderFile);


    setFloat("rcjump", temp_phys.RCJump);
    setFloat("rcstatic", temp_phys.RCStatic);
    setFloat("rcsliding", temp_phys.RCSliding);
    setFloat("rcroll", temp_phys.RCRolling);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
	unsubscribe(data);
}

void MaterialPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	temp = data->Appearance;
	temp_phys = data->Physics;
	LoadData();
}

void MaterialPropertiesDialog::OnAddTextureLevel()
{
	if (temp.NumTextureLevels >= MATERIAL_MAX_TEXTURES){
		ed->errorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.TextureFile[temp.NumTextureLevels ++] = "";
	ApplyData();
}

void MaterialPropertiesDialog::OnTextures()
{
	int sel = getInt("");
	if ((sel >= 0) && (sel <temp.NumTextureLevels))
		if (ed->fileDialog(FDTexture, false, true)){
			temp.TextureFile[sel] = ed->DialogFile;
			ApplyData();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void MaterialPropertiesDialog::OnTexturesSelect()
{
	int sel = getInt("");
	enable("mat_delete_texture_level", (sel >= 0) && (sel < temp.NumTextureLevels));
	enable("mat_empty_texture_level", (sel >= 0) && (sel < temp.NumTextureLevels));
}

void MaterialPropertiesDialog::OnDeleteTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		for (int i=sel;i<temp.NumTextureLevels-1;i++)
			temp.TextureFile[i] = temp.TextureFile[i + 1];
		temp.NumTextureLevels --;
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnEmptyTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		temp.TextureFile[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnTransparencyMode()
{
	if (isChecked("transparency_mode:function"))
		temp.TransparencyMode = TransparencyModeFunctions;
	else if (isChecked("transparency_mode:color_key"))
		temp.TransparencyMode = TransparencyModeColorKeyHard;
	else if (isChecked("transparency_mode:factor"))
		temp.TransparencyMode = TransparencyModeFactor;
	else
		temp.TransparencyMode = TransparencyModeNone;
	enable("alpha_factor", temp.TransparencyMode == TransparencyModeFactor);
	enable("alpha_source", temp.TransparencyMode == TransparencyModeFunctions);
	enable("alpha_dest", temp.TransparencyMode == TransparencyModeFunctions);
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionMode()
{
	if (isChecked("reflection_mode:cube_static"))
		temp.ReflectionMode = ReflectionCubeMapStatic;
	else if (isChecked("reflection_mode:cube_dynamic"))
		temp.ReflectionMode = ReflectionCubeMapDynamical;
	else
		temp.ReflectionMode = ReflectionNone;
	enable("reflection_size", ((temp.ReflectionMode == ReflectionCubeMapStatic) || (temp.ReflectionMode == ReflectionCubeMapDynamical)));
	enable("reflection_textures", (temp.ReflectionMode == ReflectionCubeMapStatic));
	enable("reflection_density", (temp.ReflectionMode != ReflectionNone));
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionTextures()
{
	int sel=getInt("");
	if (ed->fileDialog(FDTexture,false,true)){
		temp.ReflectionTextureFile[sel] = ed->DialogFile;
		if ((sel==0)&&(temp.ReflectionTextureFile[0].find(".") >= 0)){
			int p=temp.ReflectionTextureFile[0].find(".");
			for (int i=1;i<6;i++){
				string tf;
				tf = temp.ReflectionTextureFile[0];
				tf[p-1]=temp.ReflectionTextureFile[0][p-1]+i;
				if (file_test_existence(NixTextureDir + tf)){
					temp.ReflectionTextureFile[i] = tf;
				}
			}

		}
		ApplyData();
		RefillReflTexView();
	}
}

bool TestShaderFile(const string &filename)
{
	NixShader *shader = NixLoadShader(filename);
	shader->unref();
	return shader;
}

void MaterialPropertiesDialog::OnFindShader()
{
	if (ed->fileDialog(FDShaderFile,false,true)){
		if (TestShaderFile(ed->DialogFile)){
			setString("shader_file", ed->DialogFile);
			ApplyData();
		}else{
			ed->errorBox(_("Fehler in der Shader-Datei:\n") + NixShaderError);
		}
	}
}

void MaterialPropertiesDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	temp.ColorAmbient = win->getColor("mat_am");
	temp.ColorDiffuse = win->getColor("mat_di");
	temp.ColorSpecular = win->getColor("mat_sp");
	temp.ColorEmissive = win->getColor("mat_em");
	temp.ColorShininess = getFloat("mat_shininess");
	temp.AlphaZBuffer = isChecked("alpha_z_buffer");
	temp.AlphaFactor = getFloat("alpha_factor") * 0.01f;
	temp.AlphaSource = getInt("alpha_source");
	temp.AlphaDestination = getInt("alpha_dest");

	temp.ReflectionDensity = getInt("reflection_density");
	temp.ReflectionSize = getInt("reflection_size");

	temp.ShaderFile = getString("shader_file");

	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::ApplyDataDelayed()
{
	apply_queue_depth ++;
	HuiRunLaterM(0.5f, this, &MaterialPropertiesDialog::ApplyData);
}

void MaterialPropertiesDialog::ApplyPhysData()
{
	if (apply_phys_queue_depth> 0)
		apply_phys_queue_depth --;
	if (apply_phys_queue_depth > 0)
		return;
	temp_phys.RCJump = getFloat("rcjump");
	temp_phys.RCStatic = getFloat("rcstatic");
	temp_phys.RCSliding = getFloat("rcsliding");
	temp_phys.RCRolling = getFloat("rcroll");

	data->execute(new ActionMaterialEditPhysics(temp_phys));
}

void MaterialPropertiesDialog::ApplyPhysDataDelayed()
{
	apply_phys_queue_depth ++;
	HuiRunLaterM(0.5f, this, &MaterialPropertiesDialog::ApplyPhysData);
}

void MaterialPropertiesDialog::RefillReflTexView()
{
	reset("reflection_textures");
	addString("reflection_textures", " + X\\" + temp.ReflectionTextureFile[0]);
	addString("reflection_textures", " - X\\" + temp.ReflectionTextureFile[1]);
	addString("reflection_textures", " + Y\\" + temp.ReflectionTextureFile[2]);
	addString("reflection_textures", " - Y\\" + temp.ReflectionTextureFile[3]);
	addString("reflection_textures", " + Z\\" + temp.ReflectionTextureFile[4]);
	addString("reflection_textures", " - Z\\" + temp.ReflectionTextureFile[5]);
}

void MaterialPropertiesDialog::FillTextureList()
{
	reset("mat_textures");
	for (int i=0;i<temp.NumTextureLevels;i++){
		NixTexture *tex = NixLoadTexture(temp.TextureFile[i]);
		string img = ed->get_tex_image(tex);
		addString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.TextureFile[i]).c_str()));
	}
	if (temp.NumTextureLevels == 0)
		addString("mat_textures", _("\\\\   - keine Texturen -"));
	enable("mat_delete_texture_level", false);
	enable("mat_empty_texture_level", false);
}
