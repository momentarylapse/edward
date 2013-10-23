#include "material.h"
#include "model.h"
#include "../lib/file/file.h"
#include "../lib/nix/nix.h"
#ifdef _X_ALLOW_X_
#include "../meta.h"
#endif

color file_read_color4i(CFile *f); // -> model.cpp


string MaterialDir;

// materials
static Array<Material*> Materials;

void MaterialInit()
{
	// create the default material
	Material *m = new Material;
	m->name = "-default-";
	Materials.add(m);
}

void MaterialEnd()
{
	delete(Materials[0]);
}

void MaterialReset()
{
	// delete materials
	for (int i=1;i<Materials.num;i++)
		delete(Materials[i]);
	Materials.resize(1);
}



Material::Material()
{
	// default values
	num_textures = 0;
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		texture[i] = -1;
	cube_map = -1;
	shader = -1;

	ambient = White;
	diffuse = White;
	specular = Black;
	shininess = 0;
	emission = Black;
	transparency_mode = TransparencyModeNone;
	alpha_source = 0;
	alpha_destination = 0;
	alpha_factor = 1;
	alpha_z_buffer = true;
	reflection_mode = ReflectionNone;
	reflection_density = 0;
	cube_map_size = 0;
	rc_jump = 0.5f;
	rc_static = 0.8f;
	rc_sliding = 0.4f;
	rc_rolling = 0.90f;
}

Material::~Material()
{

}


static bool _alpha_enabled_ = false;
static bool _shader_prog_used_ = false;

void Material::apply()
{
	NixSetMaterial(ambient, diffuse, specular, shininess, emission);
	if ((shader >= 0) || (_shader_prog_used_)){
		NixSetShader(shader);
		_shader_prog_used_ = (shader >= 0);
	}

	if (transparency_mode > 0){
		if (transparency_mode == TransparencyModeFunctions)
			NixSetAlpha(alpha_source, alpha_destination);
		else if (transparency_mode == TransparencyModeFactor)
			NixSetAlpha(alpha_factor);
		else if (transparency_mode == TransparencyModeColorKeyHard)
			NixSetAlpha(AlphaColorKeyHard);
		else if (transparency_mode == TransparencyModeColorKeySmooth)
			NixSetAlpha(AlphaColorKeySmooth);
		_alpha_enabled_ = true;
	}else if (_alpha_enabled_){
		NixSetAlpha(AlphaNone);
		_alpha_enabled_ = false;
	}
	if (cube_map >= 0){
		// evil hack
		texture[3] = cube_map;
		NixSetTextures(texture, 4);
	}else
		NixSetTextures(texture, num_textures);
}

void Material::copy_from(Model *model, Material *m2, bool user_colors)
{
	if (!user_colors){
		ambient = m2->ambient;
		diffuse = m2->diffuse;
		specular = m2->specular;
		emission = m2->emission;
		shininess = m2->shininess;
	}
	int nt = num_textures;
	if (nt > m2->num_textures)
		nt = m2->num_textures;
	for (int i=0;i<nt;i++)
		if (texture[i] < 0)
			texture[i] = m2->texture[i];
	if (transparency_mode == TransparencyModeDefault){
		transparency_mode = m2->transparency_mode;
		alpha_source = m2->alpha_source;
		alpha_destination = m2->alpha_destination;
		alpha_factor = m2->alpha_factor;
		alpha_z_buffer = m2->alpha_z_buffer;
	}
	reflection_mode = m2->reflection_mode;
	reflection_density = m2->reflection_density;
	cube_map = m2->cube_map;
/*	if ((cube_map < 0) && (m2->cube_map_size > 0) && (reflection_mode == ReflectionCubeMapDynamical)){
		cube_map = FxCubeMapNew(m2->cube_map_size);
		FxCubeMapCreate(cube_map, model);
	}*/
	shader = m2->shader;
	rc_static = m2->rc_static;
	rc_sliding = m2->rc_sliding;
	rc_jump = m2->rc_jump;
	rc_rolling = m2->rc_rolling;
}


Material *LoadMaterial(const string &filename, bool as_default)
{
	// an empty name loads the default material
	if (filename.num == 0)
		return Materials[0];

	if (!as_default){
		for (int i=0;i<Materials.num;i++)
			if (Materials[i]->name == filename)
				return Materials[i];
	}
	CFile *f = FileOpen(MaterialDir + filename + ".material");
	if (!f){
#ifdef _X_ALLOW_X_
	if (Engine.FileErrorsAreCritical)
		return NULL;
#endif
		return Materials[0];
	}
	Material *m = new Material;

	int ffv = f->ReadFileFormatVersion();
	if (ffv == 4){
		m->name = filename;
		// Textures
		m->num_textures = f->ReadIntC();
		for (int i=0;i<m->num_textures;i++)
			m->texture[i] = NixLoadTexture(f->ReadStr());
		// Colors
		f->ReadComment();
		m->ambient = file_read_color4i(f);
		m->diffuse = file_read_color4i(f);
		m->specular = file_read_color4i(f);
		m->shininess = (float)f->ReadInt();
		m->emission = file_read_color4i(f);
		// Transparency
		m->transparency_mode = f->ReadIntC();
		m->alpha_factor = float(f->ReadInt()) * 0.01f;
		m->alpha_source = f->ReadInt();
		m->alpha_destination = f->ReadInt();
		m->alpha_z_buffer = f->ReadBool();
		// Appearance
		f->ReadIntC(); //ShiningDensity
		f->ReadInt(); // ShiningLength
		f->ReadBool(); // IsWater
		// Reflection
		m->cube_map = -1;
		m->cube_map_size = 0;
		m->reflection_density = 0;
		m->reflection_mode = f->ReadIntC();
		m->reflection_density = float(f->ReadInt()) * 0.01f;
		m->cube_map_size = f->ReadInt();
		int cmt[6];
		for (int i=0;i<6;i++)
			cmt[i] = NixLoadTexture(f->ReadStr());
		if (m->reflection_mode == ReflectionCubeMapDynamical){
			//m->cube_map = FxCubeMapNew(m->cube_map_size);
			//FxCubeMapCreate(m->cube_map,cmt[0],cmt[1],cmt[2],cmt[3],cmt[4],cmt[5]);
		}else if (m->reflection_mode == ReflectionCubeMapStatic){
			m->cube_map = NixCreateCubeMap(m->cube_map_size);
			for (int i=0;i<6;i++)
				NixFillCubeMap(m->cube_map, i, cmt[i]);
		}
		// ShaderFile
		string ShaderFile = f->ReadStrC();
		m->shader = NixLoadShader(ShaderFile);
		// Physics
		m->rc_jump = (float)f->ReadIntC() * 0.001f;
		m->rc_static = (float)f->ReadInt() * 0.001f;
		m->rc_sliding = (float)f->ReadInt() * 0.001f;
		m->rc_rolling = (float)f->ReadInt() * 0.001f;

		Materials.add(m);
	}else{
		msg_error(format("wrong file format: %d (expected: 4)", ffv));
		m = Materials[0];
	}
	FileClose(f);
	return m;
}
