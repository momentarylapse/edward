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

namespace nix{
	extern string shader_error; // -> nix
};

MaterialPropertiesDialog::MaterialPropertiesDialog(hui::Window *_parent, DataMaterial *_data):
	Observer("MaterialPropertiesDialog")
{
	fromResource("material_dialog");
	data = _data;

	// dialog
	event("mat_add_texture_level", std::bind(&MaterialPropertiesDialog::OnAddTextureLevel, this));
	event("mat_textures", std::bind(&MaterialPropertiesDialog::OnTextures, this));
	eventX("mat_textures", "hui:select", std::bind(&MaterialPropertiesDialog::OnTexturesSelect, this));
	event("mat_delete_texture_level", std::bind(&MaterialPropertiesDialog::OnDeleteTextureLevel, this));
	event("mat_empty_texture_level", std::bind(&MaterialPropertiesDialog::OnEmptyTextureLevel, this));
	event("transparency_mode:none", std::bind(&MaterialPropertiesDialog::OnTransparencyMode, this));
	event("transparency_mode:function", std::bind(&MaterialPropertiesDialog::OnTransparencyMode, this));
	event("transparency_mode:color_key", std::bind(&MaterialPropertiesDialog::OnTransparencyMode, this));
	event("transparency_mode:factor", std::bind(&MaterialPropertiesDialog::OnTransparencyMode, this));
	event("reflection_mode:none", std::bind(&MaterialPropertiesDialog::OnReflectionMode, this));
	event("reflection_mode:cube_static", std::bind(&MaterialPropertiesDialog::OnReflectionMode, this));
	event("reflection_mode:cube_dynamic", std::bind(&MaterialPropertiesDialog::OnReflectionMode, this));
	event("reflection_textures", std::bind(&MaterialPropertiesDialog::OnReflectionTextures, this));
	event("find_shader", std::bind(&MaterialPropertiesDialog::OnFindShader, this));


	event("mat_am", std::bind(&MaterialPropertiesDialog::ApplyData, this));
	event("mat_di", std::bind(&MaterialPropertiesDialog::ApplyData, this));
	event("mat_sp", std::bind(&MaterialPropertiesDialog::ApplyData, this));
	event("mat_em", std::bind(&MaterialPropertiesDialog::ApplyData, this));
	event("mat_shininess", std::bind(&MaterialPropertiesDialog::ApplyDataDelayed, this));

	event("alpha_factor", std::bind(&MaterialPropertiesDialog::ApplyDataDelayed, this));
	event("alpha_source", std::bind(&MaterialPropertiesDialog::ApplyDataDelayed, this));
	event("alpha_dest", std::bind(&MaterialPropertiesDialog::ApplyDataDelayed, this));
	event("alpha_z_buffer", std::bind(&MaterialPropertiesDialog::ApplyData, this));

	event("rcjump", std::bind(&MaterialPropertiesDialog::ApplyPhysDataDelayed, this));
	event("rcstatic", std::bind(&MaterialPropertiesDialog::ApplyPhysDataDelayed, this));
	event("rcsliding", std::bind(&MaterialPropertiesDialog::ApplyPhysDataDelayed, this));
	event("rcroll", std::bind(&MaterialPropertiesDialog::ApplyPhysDataDelayed, this));

	expand("material_dialog_grp_color", 0, true);

	temp = data->appearance;
	temp_phys = data->physics;
	apply_queue_depth = 0;
	apply_phys_queue_depth = 0;
	LoadData();
	subscribe(data);
}

void MaterialPropertiesDialog::LoadData()
{
	FillTextureList();
	setColor("mat_am", temp.ambient);
	setColor("mat_di", temp.diffuse);
	setColor("mat_sp", temp.specular);
	setColor("mat_em", temp.emissive);
	setFloat("mat_shininess", temp.shininess);

	if (temp.transparency_mode == TransparencyModeColorKeySmooth)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyModeColorKeyHard)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TransparencyModeFactor)
		check("transparency_mode:factor", true);
	else if (temp.transparency_mode == TransparencyModeFunctions)
		check("transparency_mode:function", true);
	else
		check("transparency_mode:none", true);
	enable("alpha_factor", temp.transparency_mode == TransparencyModeFactor);
	enable("alpha_source", temp.transparency_mode == TransparencyModeFunctions);
	enable("alpha_dest", temp.transparency_mode == TransparencyModeFunctions);
	setFloat("alpha_factor", temp.alpha_factor * 100.0f);
	check("alpha_z_buffer", temp.alpha_z_buffer);
	setInt("alpha_source", temp.alpha_source);
	setInt("alpha_dest", temp.alpha_destination);

	if (temp.reflection_mode == ReflectionCubeMapStatic)
		check("reflection_mode:cube_static", true);
	else if (temp.reflection_mode == ReflectionCubeMapDynamical)
		check("reflection_mode:cube_dynamic", true);
	else
		check("reflection_mode:none", true);
	setInt("reflection_size", temp.reflection_size);
	setInt("reflection_density", temp.reflection_density);
	RefillReflTexView();
	enable("reflection_size", ((temp.reflection_mode == ReflectionCubeMapStatic) || (temp.reflection_mode == ReflectionCubeMapDynamical)));
	enable("reflection_textures", (temp.reflection_mode == ReflectionCubeMapStatic));
	enable("reflection_density", (temp.reflection_mode != ReflectionNone));
	setString("shader_file", temp.shader_file);


    setFloat("rcjump", temp_phys.friction_jump);
    setFloat("rcstatic", temp_phys.friction_static);
    setFloat("rcsliding", temp_phys.friction_sliding);
    setFloat("rcroll", temp_phys.friction_rolling);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
	unsubscribe(data);
}

void MaterialPropertiesDialog::onUpdate(Observable *o, const string &message)
{
	temp = data->appearance;
	temp_phys = data->physics;
	LoadData();
}

void MaterialPropertiesDialog::OnAddTextureLevel()
{
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		ed->errorBox(format(_("H&ochstens %d Textur-Ebenen erlaubt!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	ApplyData();
}

void MaterialPropertiesDialog::OnTextures()
{
	int sel = getInt("");
	if ((sel >= 0) and (sel < temp.texture_files.num))
		if (ed->fileDialog(FD_TEXTURE, false, true)){
			temp.texture_files[sel] = ed->dialog_file;
			ApplyData();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void MaterialPropertiesDialog::OnTexturesSelect()
{
	int sel = getInt("");
	enable("mat_delete_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
	enable("mat_empty_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
}

void MaterialPropertiesDialog::OnDeleteTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		temp.texture_files.erase(sel);
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnEmptyTextureLevel()
{
	int sel = getInt("mat_textures");
	if (sel >= 0){
		temp.texture_files[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnTransparencyMode()
{
	if (isChecked("transparency_mode:function"))
		temp.transparency_mode = TransparencyModeFunctions;
	else if (isChecked("transparency_mode:color_key"))
		temp.transparency_mode = TransparencyModeColorKeyHard;
	else if (isChecked("transparency_mode:factor"))
		temp.transparency_mode = TransparencyModeFactor;
	else
		temp.transparency_mode = TransparencyModeNone;
	enable("alpha_factor", temp.transparency_mode == TransparencyModeFactor);
	enable("alpha_source", temp.transparency_mode == TransparencyModeFunctions);
	enable("alpha_dest", temp.transparency_mode == TransparencyModeFunctions);
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionMode()
{
	if (isChecked("reflection_mode:cube_static"))
		temp.reflection_mode = ReflectionCubeMapStatic;
	else if (isChecked("reflection_mode:cube_dynamic"))
		temp.reflection_mode = ReflectionCubeMapDynamical;
	else
		temp.reflection_mode = ReflectionNone;
	enable("reflection_size", ((temp.reflection_mode == ReflectionCubeMapStatic) || (temp.reflection_mode == ReflectionCubeMapDynamical)));
	enable("reflection_textures", (temp.reflection_mode == ReflectionCubeMapStatic));
	enable("reflection_density", (temp.reflection_mode != ReflectionNone));
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionTextures()
{
	int sel=getInt("");
	if (ed->fileDialog(FD_TEXTURE,false,true)){
		temp.reflection_texture_file[sel] = ed->dialog_file;
		if ((sel==0) and (temp.reflection_texture_file[0].find(".") >= 0)){
			int p=temp.reflection_texture_file[0].find(".");
			for (int i=1;i<6;i++){
				string tf;
				tf = temp.reflection_texture_file[0];
				tf[p-1]=temp.reflection_texture_file[0][p-1]+i;
				if (file_test_existence(nix::texture_dir + tf)){
					temp.reflection_texture_file[i] = tf;
				}
			}

		}
		ApplyData();
		RefillReflTexView();
	}
}

bool TestShaderFile(const string &filename)
{
	nix::Shader *shader = nix::LoadShader(filename);
	shader->unref();
	return shader;
}

void MaterialPropertiesDialog::OnFindShader()
{
	if (ed->fileDialog(FD_SHADERFILE,false,true)){
		if (TestShaderFile(ed->dialog_file)){
			setString("shader_file", ed->dialog_file);
			ApplyData();
		}else{
			ed->errorBox(_("Fehler in der Shader-Datei:\n") + nix::shader_error);
		}
	}
}

void MaterialPropertiesDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	temp.ambient = getColor("mat_am");
	temp.diffuse = getColor("mat_di");
	temp.specular = getColor("mat_sp");
	temp.emissive = getColor("mat_em");
	temp.shininess = getFloat("mat_shininess");
	temp.alpha_z_buffer = isChecked("alpha_z_buffer");
	temp.alpha_factor = getFloat("alpha_factor") * 0.01f;
	temp.alpha_source = getInt("alpha_source");
	temp.alpha_destination = getInt("alpha_dest");

	temp.reflection_density = getInt("reflection_density");
	temp.reflection_size = getInt("reflection_size");

	temp.shader_file = getString("shader_file");

	data->execute(new ActionMaterialEditAppearance(temp));
}

void MaterialPropertiesDialog::ApplyDataDelayed()
{
	apply_queue_depth ++;
	hui::RunLater(0.5f, std::bind(&MaterialPropertiesDialog::ApplyData, this));
}

void MaterialPropertiesDialog::ApplyPhysData()
{
	if (apply_phys_queue_depth> 0)
		apply_phys_queue_depth --;
	if (apply_phys_queue_depth > 0)
		return;
	temp_phys.friction_jump = getFloat("rcjump");
	temp_phys.friction_static = getFloat("rcstatic");
	temp_phys.friction_sliding = getFloat("rcsliding");
	temp_phys.friction_rolling = getFloat("rcroll");

	data->execute(new ActionMaterialEditPhysics(temp_phys));
}

void MaterialPropertiesDialog::ApplyPhysDataDelayed()
{
	apply_phys_queue_depth ++;
	hui::RunLater(0.5f, std::bind(&MaterialPropertiesDialog::ApplyPhysData, this));
}

void MaterialPropertiesDialog::RefillReflTexView()
{
	reset("reflection_textures");
	addString("reflection_textures", " + X\\" + temp.reflection_texture_file[0]);
	addString("reflection_textures", " - X\\" + temp.reflection_texture_file[1]);
	addString("reflection_textures", " + Y\\" + temp.reflection_texture_file[2]);
	addString("reflection_textures", " - Y\\" + temp.reflection_texture_file[3]);
	addString("reflection_textures", " + Z\\" + temp.reflection_texture_file[4]);
	addString("reflection_textures", " - Z\\" + temp.reflection_texture_file[5]);
}

void MaterialPropertiesDialog::FillTextureList()
{
	reset("mat_textures");
	for (int i=0;i<temp.texture_files.num;i++){
		nix::Texture *tex = nix::LoadTexture(temp.texture_files[i]);
		string img = ed->get_tex_image(tex);
		addString("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.texture_files[i]).c_str()));
	}
	if (temp.texture_files.num == 0)
		addString("mat_textures", _("\\\\   - keine Texturen -"));
	enable("mat_delete_texture_level", false);
	enable("mat_empty_texture_level", false);
}
