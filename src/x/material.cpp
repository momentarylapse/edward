#include "material.h"
#include "model.h"
#include "../lib/file/file.h"
#include "../lib/nix/nix.h"
#ifdef _X_ALLOW_X_
#include "../meta.h"
#endif

color file_read_color4i(File *f); // -> model.cpp


string MaterialDir;

// materials
static Material *default_material;
static Material *trivial_material;
static Array<Material*> materials;

void MaterialInit()
{
	// create the default material
	trivial_material = new Material;
	trivial_material->name = "-default-";

	SetDefaultMaterial(trivial_material);
}

void MaterialEnd()
{
	delete(trivial_material);
}

void MaterialReset()
{
	for (auto *m: materials)
		delete(m);
	materials.clear();

	SetDefaultMaterial(trivial_material);
}


void SetDefaultMaterial(Material *m)
{
	default_material = m;
}



Material::Material()
{
	// default values
	cube_map = NULL;
	shader = NULL;

	ambient = White;
	diffuse = White;
	specular = Black;
	shininess = 0;
	emission = Black;

	transparency_mode = TRANSPARENCY_NONE;
	alpha_source = 0;
	alpha_destination = 0;
	alpha_factor = 1;
	alpha_z_buffer = true;

	reflection_mode = REFLECTION_NONE;
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

void Material::apply()
{
	nix::SetMaterial(ambient, diffuse, specular, shininess, emission);
	nix::SetShader(shader);

	if (transparency_mode > 0){
		if (transparency_mode == TRANSPARENCY_FUNCTIONS)
			nix::SetAlpha(alpha_source, alpha_destination);
		else if (transparency_mode == TRANSPARENCY_FACTOR)
			nix::SetAlpha(alpha_factor);
		else if (transparency_mode == TRANSPARENCY_COLOR_KEY_HARD)
			nix::SetAlpha(ALPHA_COLOR_KEY_HARD);
		else if (transparency_mode == TRANSPARENCY_COLOR_KEY_SMOOTH)
			nix::SetAlpha(ALPHA_COLOR_KEY_SMOOTH);
		_alpha_enabled_ = true;
	}else if (_alpha_enabled_){
		nix::SetAlpha(ALPHA_NONE);
		_alpha_enabled_ = false;
	}
	if (cube_map){
		Array<nix::Texture*> tex = textures;
		tex.add(cube_map);
		nix::SetTextures(tex);
	}else
		nix::SetTextures(textures);
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
	int nt = textures.num;
	if (nt > m2->textures.num)
		nt = m2->textures.num;
	for (int i=0;i<nt;i++)
		if (!textures[i])
			textures[i] = m2->textures[i];
	if (transparency_mode == TRANSPARENCY_DEFAULT){
		transparency_mode = m2->transparency_mode;
		alpha_source = m2->alpha_source;
		alpha_destination = m2->alpha_destination;
		alpha_factor = m2->alpha_factor;
		alpha_z_buffer = m2->alpha_z_buffer;
	}
	reflection_mode = m2->reflection_mode;
	reflection_density = m2->reflection_density;
	cube_map = m2->cube_map;
/*	if ((cube_map < 0) and (m2->cube_map_size > 0) and (reflection_mode == ReflectionCubeMapDynamical)){
		cube_map = FxCubeMapNew(m2->cube_map_size);
		FxCubeMapCreate(cube_map, model);
	}*/
	shader = m2->shader;
	rc_static = m2->rc_static;
	rc_sliding = m2->rc_sliding;
	rc_jump = m2->rc_jump;
	rc_rolling = m2->rc_rolling;
}


Material *LoadMaterial(const string &filename)
{
	// an empty name loads the default material
	if (filename.num == 0)
		return new Material;//default_material;

	File *f;
	msg_write("loading material " + filename);
	try{
		f = FileOpenText(MaterialDir + filename + ".material");
	}catch(FileError &e){
#ifdef _X_ALLOW_X_
		msg_error(e.message());
#endif
		return default_material;
	}
	Material *m = new Material;

	int ffv = f->ReadFileFormatVersion();
	if (ffv == 4){
		m->name = filename;
		// Textures
		f->read_comment();
		int nt = f->read_int();
		m->textures.resize(nt);
		for (int i=0;i<nt;i++)
			m->textures[i] = nix::LoadTexture(f->read_str());
		// Colors
		f->read_comment();
		m->ambient = file_read_color4i(f);
		m->diffuse = file_read_color4i(f);
		m->specular = file_read_color4i(f);
		m->shininess = (float)f->read_int();
		m->emission = file_read_color4i(f);
		// Transparency
		f->read_comment();
		m->transparency_mode = f->read_int();
		m->alpha_factor = float(f->read_int()) * 0.01f;
		m->alpha_source = f->read_int();
		m->alpha_destination = f->read_int();
		m->alpha_z_buffer = f->read_bool();
		// Appearance
		f->read_comment();
		f->read_int(); //ShiningDensity
		f->read_int(); // ShiningLength
		f->read_bool(); // IsWater
		// Reflection
		m->cube_map = NULL;
		m->cube_map_size = 0;
		m->reflection_density = 0;
		f->read_comment();
		m->reflection_mode = f->read_int();
		m->reflection_density = float(f->read_int()) * 0.01f;
		m->cube_map_size = f->read_int();
		nix::Texture *cmt[6];
		for (int i=0;i<6;i++)
			cmt[i] = nix::LoadTexture(f->read_str());
		if (m->reflection_mode == REFLECTION_CUBE_MAP_DYNAMIC){
			//m->cube_map = FxCubeMapNew(m->cube_map_size);
			//FxCubeMapCreate(m->cube_map,cmt[0],cmt[1],cmt[2],cmt[3],cmt[4],cmt[5]);
		}else if (m->reflection_mode == REFLECTION_CUBE_MAP_STATIC){
			m->cube_map = new nix::CubeMap(m->cube_map_size);
			for (int i=0;i<6;i++)
				m->cube_map->fill_side(i, cmt[i]);
		}
		// ShaderFile
		f->read_comment();
		string ShaderFile = f->read_str();
		m->shader = nix::LoadShader(ShaderFile);
		// Physics
		f->read_comment();
		m->rc_jump = (float)f->read_int() * 0.001f;
		m->rc_static = (float)f->read_int() * 0.001f;
		m->rc_sliding = (float)f->read_int() * 0.001f;
		m->rc_rolling = (float)f->read_int() * 0.001f;

		materials.add(m);
	}else{
		msg_error(format("wrong file format: %d (expected: 4)", ffv));
		m = default_material;
	}
	FileClose(f);
	return m;
}
