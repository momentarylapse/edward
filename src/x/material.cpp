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
static Array<Material*> materials; // "originals"

void MaterialInit() {
	// create the default material
	trivial_material = new Material;
	trivial_material->name = "-default-";

	SetDefaultMaterial(trivial_material);
}

void MaterialEnd() {
	delete trivial_material;
}

void MaterialReset() {
	for (auto *m: materials)
		delete m;
	materials.clear();

	SetDefaultMaterial(trivial_material);
}


void SetDefaultMaterial(Material *m) {
	default_material = m;
}



Material::Material() {
	// default values
	reflection.cube_map = NULL;
	shader = NULL;

	ambient = White;
	diffuse = White;
	specular = Black;
	shininess = 0;
	emission = Black;

	alpha.mode = TRANSPARENCY_NONE;
	alpha.source = 0;
	alpha.destination = 0;
	alpha.factor = 1;
	alpha.z_buffer = true;

	reflection.mode = REFLECTION_NONE;
	reflection.density = 0;
	reflection.cube_map_size = 0;

	friction.jump = 0.5f;
	friction._static = 0.8f;
	friction.sliding = 0.4f;
	friction.rolling = 0.90f;
}

Material::~Material() {
	if (shader)
		delete shader;
	//	shader->unref();
}


Material* Material::copy() {
	Material *m = new Material;
	m->ambient = ambient;
	m->diffuse = diffuse;
	m->specular = specular;
	m->emission = emission;
	m->shininess = shininess;

	m->textures = textures;

	m->alpha = alpha;
	m->reflection = reflection;
	m->reflection.cube_map = reflection.cube_map;
/*	if ((cube_map < 0) and (m2->cube_map_size > 0) and (reflection.mode == ReflectionCubeMapDynamical)){
		cube_map = FxCubeMapNew(m2->cube_map_size);
		FxCubeMapCreate(cube_map, model);
	}*/
	m->shader = shader;
	m->friction = friction;
	return m;
}


Material *LoadMaterial(const string &filename) {
	// an empty name loads the default material
	if (filename.num == 0)
		return default_material->copy();

	for (Material *m: materials)
		if (m->name == filename)
			return m->copy();


	File *f = NULL;
	msg_write("loading material " + filename);
	try {
		msg_right();
		f = FileOpenText(MaterialDir + filename + ".material");
		Material *m = new Material;

		int ffv = f->ReadFileFormatVersion();
		if (ffv != 4)
			throw Exception(format("wrong file format: %d (expected: 4)", ffv));
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
		m->alpha.mode = f->read_int();
		m->alpha.factor = float(f->read_int()) * 0.01f;
		m->alpha.source = f->read_int();
		m->alpha.destination = f->read_int();
		m->alpha.z_buffer = f->read_bool();
		// Appearance
		f->read_comment();
		f->read_int(); //ShiningDensity
		f->read_int(); // ShiningLength
		f->read_bool(); // IsWater
		// Reflection
		m->reflection.cube_map = NULL;
		m->reflection.cube_map_size = 0;
		m->reflection.density = 0;
		f->read_comment();
		m->reflection.mode = f->read_int();
		m->reflection.density = float(f->read_int()) * 0.01f;
		m->reflection.cube_map_size = f->read_int();
		nix::Texture *cmt[6];
		for (int i=0;i<6;i++)
			cmt[i] = nix::LoadTexture(f->read_str());
		if (m->reflection.mode == REFLECTION_CUBE_MAP_DYNAMIC){
			//m->cube_map = FxCubeMapNew(m->cube_map_size);
			//FxCubeMapCreate(m->cube_map,cmt[0],cmt[1],cmt[2],cmt[3],cmt[4],cmt[5]);
		}else if (m->reflection.mode == REFLECTION_CUBE_MAP_STATIC){
#if 0
			m->reflection.cube_map = new nix::CubeMap(m->reflection.cube_map_size);
			for (int i=0;i<6;i++)
				m->reflection.cube_map->fill_side(i, cmt[i]);
#endif
		}
		// ShaderFile
		f->read_comment();
		string ShaderFile = f->read_str();
		m->shader = nix::Shader::load(ShaderFile);
		// Physics
		f->read_comment();
		m->friction.jump = (float)f->read_int() * 0.001f;
		m->friction._static = (float)f->read_int() * 0.001f;
		m->friction.sliding = (float)f->read_int() * 0.001f;
		m->friction.rolling = (float)f->read_int() * 0.001f;

		materials.add(m);
		delete f;
		msg_left();
		return m->copy();

	}catch(Exception &e){
		if (f)
			delete f;
		msg_left();
		throw e;
	}
	return NULL;
}
