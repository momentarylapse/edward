/*
 * MaterialPropertiesDialog.cpp
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#include "MaterialPropertiesDialog.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
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
	from_resource("material_dialog");
	data = _data;

	// dialog
	event("mat_add_texture_level", std::bind(&MaterialPropertiesDialog::OnAddTextureLevel, this));
	event("mat_textures", std::bind(&MaterialPropertiesDialog::OnTextures, this));
	event_x("mat_textures", "hui:select", std::bind(&MaterialPropertiesDialog::OnTexturesSelect, this));
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
	set_color("mat_am", temp.ambient);
	set_color("mat_di", temp.diffuse);
	set_color("mat_sp", temp.specular);
	set_color("mat_em", temp.emissive);
	set_float("mat_shininess", temp.shininess);

	if (temp.transparency_mode == TRANSPARENCY_COLOR_KEY_SMOOTH)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TRANSPARENCY_COLOR_KEY_HARD)
		check("transparency_mode:color_key", true);
	else if (temp.transparency_mode == TRANSPARENCY_FACTOR)
		check("transparency_mode:factor", true);
	else if (temp.transparency_mode == TRANSPARENCY_FUNCTIONS)
		check("transparency_mode:function", true);
	else
		check("transparency_mode:none", true);
	enable("alpha_factor", temp.transparency_mode == TRANSPARENCY_FACTOR);
	enable("alpha_source", temp.transparency_mode == TRANSPARENCY_FUNCTIONS);
	enable("alpha_dest", temp.transparency_mode == TRANSPARENCY_FUNCTIONS);
	set_float("alpha_factor", temp.alpha_factor * 100.0f);
	check("alpha_z_buffer", temp.alpha_z_buffer);
	set_int("alpha_source", temp.alpha_source);
	set_int("alpha_dest", temp.alpha_destination);

	if (temp.reflection_mode == REFLECTION_CUBE_MAP_STATIC)
		check("reflection_mode:cube_static", true);
	else if (temp.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC)
		check("reflection_mode:cube_dynamic", true);
	else
		check("reflection_mode:none", true);
	if (temp.reflection_size >= 512)
		set_int("reflection_size", 3);
	else if (temp.reflection_size >= 256)
		set_int("reflection_size", 2);
	else if (temp.reflection_size >= 128)
		set_int("reflection_size", 1);
	else
		set_int("reflection_size", 0);
	set_int("reflection_density", temp.reflection_density);
	RefillReflTexView();
	enable("reflection_size", ((temp.reflection_mode == REFLECTION_CUBE_MAP_STATIC) or (temp.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC)));
	enable("reflection_textures", (temp.reflection_mode == REFLECTION_CUBE_MAP_STATIC));
	enable("reflection_density", (temp.reflection_mode != REFLECTION_NONE));
	set_string("shader_file", temp.shader_file);


    set_float("rcjump", temp_phys.friction_jump);
    set_float("rcstatic", temp_phys.friction_static);
    set_float("rcsliding", temp_phys.friction_sliding);
    set_float("rcroll", temp_phys.friction_rolling);
}

MaterialPropertiesDialog::~MaterialPropertiesDialog()
{
	unsubscribe(data);
}

void MaterialPropertiesDialog::on_update(Observable *o, const string &message)
{
	temp = data->appearance;
	temp_phys = data->physics;
	LoadData();
}

void MaterialPropertiesDialog::OnAddTextureLevel()
{
	if (temp.texture_files.num >= MATERIAL_MAX_TEXTURES){
		ed->error_box(format(_("Only %d texture levels allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	temp.texture_files.add("");
	ApplyData();
}

void MaterialPropertiesDialog::OnTextures()
{
	int sel = get_int("");
	if ((sel >= 0) and (sel < temp.texture_files.num))
		if (storage->file_dialog(FD_TEXTURE, false, true)){
			temp.texture_files[sel] = storage->dialog_file;
			ApplyData();
			//mmaterial->Texture[sel] = MetaLoadTexture(mmaterial->TextureFile[sel]);
			FillTextureList();
		}
}

void MaterialPropertiesDialog::OnTexturesSelect()
{
	int sel = get_int("");
	enable("mat_delete_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
	enable("mat_empty_texture_level", (sel >= 0) and (sel < temp.texture_files.num));
}

void MaterialPropertiesDialog::OnDeleteTextureLevel()
{
	int sel = get_int("mat_textures");
	if (sel >= 0){
		temp.texture_files.erase(sel);
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnEmptyTextureLevel()
{
	int sel = get_int("mat_textures");
	if (sel >= 0){
		temp.texture_files[sel] = "";
		ApplyData();
		FillTextureList();
	}
}

void MaterialPropertiesDialog::OnTransparencyMode()
{
	if (is_checked("transparency_mode:function"))
		temp.transparency_mode = TRANSPARENCY_FUNCTIONS;
	else if (is_checked("transparency_mode:color_key"))
		temp.transparency_mode = TRANSPARENCY_COLOR_KEY_HARD;
	else if (is_checked("transparency_mode:factor"))
		temp.transparency_mode = TRANSPARENCY_FACTOR;
	else
		temp.transparency_mode = TRANSPARENCY_NONE;
	enable("alpha_factor", temp.transparency_mode == TRANSPARENCY_FACTOR);
	enable("alpha_source", temp.transparency_mode == TRANSPARENCY_FUNCTIONS);
	enable("alpha_dest", temp.transparency_mode == TRANSPARENCY_FUNCTIONS);
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionMode()
{
	if (is_checked("reflection_mode:cube_static"))
		temp.reflection_mode = REFLECTION_CUBE_MAP_STATIC;
	else if (is_checked("reflection_mode:cube_dynamic"))
		temp.reflection_mode = REFLECTION_CUBE_MAP_DYNAMIC;
	else
		temp.reflection_mode = REFLECTION_NONE;
	enable("reflection_size", ((temp.reflection_mode == REFLECTION_CUBE_MAP_STATIC) or (temp.reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC)));
	enable("reflection_textures", (temp.reflection_mode == REFLECTION_CUBE_MAP_STATIC));
	enable("reflection_density", (temp.reflection_mode != REFLECTION_NONE));
	ApplyData();
}

void MaterialPropertiesDialog::OnReflectionTextures()
{
	int sel=get_int("");
	if (storage->file_dialog(FD_TEXTURE,false,true)){
		temp.reflection_texture_file[sel] = storage->dialog_file;
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
	nix::Shader *shader = nix::Shader::load(filename);
	shader->unref();
	return shader;
}

void MaterialPropertiesDialog::OnFindShader()
{
	if (storage->file_dialog(FD_SHADERFILE,false,true)){
		if (TestShaderFile(storage->dialog_file)){
			set_string("shader_file", storage->dialog_file);
			ApplyData();
		}else{
			ed->error_box(_("Error in shader file:\n") + nix::shader_error);
		}
	}
}

void MaterialPropertiesDialog::ApplyData()
{
	if (apply_queue_depth > 0)
		apply_queue_depth --;
	if (apply_queue_depth > 0)
		return;
	temp.ambient = get_color("mat_am");
	temp.diffuse = get_color("mat_di");
	temp.specular = get_color("mat_sp");
	temp.emissive = get_color("mat_em");
	temp.shininess = get_float("mat_shininess");
	temp.alpha_z_buffer = is_checked("alpha_z_buffer");
	temp.alpha_factor = get_float("alpha_factor") * 0.01f;
	temp.alpha_source = get_int("alpha_source");
	temp.alpha_destination = get_int("alpha_dest");

	temp.reflection_density = get_int("reflection_density");
	temp.reflection_size = 64 << get_int("reflection_size");

	temp.shader_file = get_string("shader_file");

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
	temp_phys.friction_jump = get_float("rcjump");
	temp_phys.friction_static = get_float("rcstatic");
	temp_phys.friction_sliding = get_float("rcsliding");
	temp_phys.friction_rolling = get_float("rcroll");

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
	add_string("reflection_textures", " + X\\" + temp.reflection_texture_file[0]);
	add_string("reflection_textures", " - X\\" + temp.reflection_texture_file[1]);
	add_string("reflection_textures", " + Y\\" + temp.reflection_texture_file[2]);
	add_string("reflection_textures", " - Y\\" + temp.reflection_texture_file[3]);
	add_string("reflection_textures", " + Z\\" + temp.reflection_texture_file[4]);
	add_string("reflection_textures", " - Z\\" + temp.reflection_texture_file[5]);
}

void MaterialPropertiesDialog::FillTextureList()
{
	reset("mat_textures");
	for (int i=0;i<temp.texture_files.num;i++){
		nix::Texture *tex = nix::LoadTexture(temp.texture_files[i]);
		string img = ed->get_tex_image(tex);
		add_string("mat_textures", format("Tex[%d]\\%s\\%s", i, img.c_str(), file_secure(temp.texture_files[i]).c_str()));
	}
	if (temp.texture_files.num == 0)
		add_string("mat_textures", _("\\\\   - no textures -"));
	enable("mat_delete_texture_level", false);
	enable("mat_empty_texture_level", false);
}
