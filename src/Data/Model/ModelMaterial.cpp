/*
 * ModelMaterial.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModelMaterial.h"
#include "../../lib/nix/nix.h"
#include "../../Edward.h"


ModelMaterial::ModelMaterial()
{
	vb = NULL;
	reset();
}

ModelMaterial::ModelMaterial(const string &filename)
{
	vb = NULL;
	reset();
	material_file = filename;
	makeConsistent();
}

ModelMaterial::~ModelMaterial()
{
	if (vb)
		delete(vb);
	vb = NULL;
}

void ModelMaterial::reset()
{
	// transparency
	user_transparency = false;
	transparency_mode = TransparencyModeDefault;
	alpha_destination = 0;
	alpha_source = 0;
	alpha_factor = 50;
	alpha_zbuffer = true;

	// color
	user_color = false;
	ambient = White;
	diffuse = White;
	specular = Black;
	emission = Black;
	shininess = 20;

	// file
	material_file = "";
	material = LoadMaterial("");

	// textures
	textures = NULL;
	texture_files.clear();
	texture_files.add("");

	if (vb)
		delete(vb);
	vb = NULL;
}

void ModelMaterial::operator =(const ModelMaterial &m)
{
	user_transparency = m.user_transparency;
	transparency_mode = m.transparency_mode;
	alpha_destination = m.alpha_destination;
	alpha_source = m.alpha_source;
	alpha_factor = m.alpha_factor;
	alpha_zbuffer = m.alpha_zbuffer;

	user_color = m.user_color;
	ambient = m.ambient;
	diffuse = m.diffuse;
	specular = m.specular;
	emission = m.emission;
	shininess = m.shininess;

	material_file = m.material_file;
	material = m.material;

	texture_files = m.texture_files;
	textures = m.textures;
}

void ModelMaterial::makeConsistent()
{
	material = LoadMaterial(material_file);
	checkTextures();
	checkTransparency();
	checkColors();
}

void ModelMaterial::checkTransparency()
{
	if (transparency_mode == TransparencyModeDefault)
		user_transparency = false;
	if (!user_transparency){
		transparency_mode = material->transparency_mode;
		alpha_source = material->alpha_source;
		alpha_destination = material->alpha_destination;
		alpha_factor	= material->alpha_factor;
		alpha_zbuffer = material->alpha_z_buffer;
	}
}



void ModelMaterial::checkTextures()
{
	// parent has more texture levels?
	if (material->textures.num > texture_files.num){
		texture_files.resize(material->textures.num);
		ed->setMessage(_("Anzahl der Texturen wurde an das Material angepasst!"));
	}

	// load all textures
	textures.clear();
	for (string &tf: texture_files)
		textures.add(nix::LoadTexture(tf));

	// parent overwrites unused textures
	for (int i=0;i<material->textures.num;i++)
		if (!textures[i])
			if (texture_files[i] == "")
				textures[i] = material->textures[i];
}

void ModelMaterial::checkColors()
{
	if (!user_color){
		ambient = material->ambient;
		diffuse = material->diffuse;
		specular = material->specular;
		shininess = material->shininess;
		emission = material->emission;
	}
}

void ModelMaterial::applyForRendering()
{
	nix::SetAlpha(AlphaNone);
	nix::SetShader(nix::default_shader_3d);
	color em = ColorInterpolate(emission, White, 0.1f);
	nix::SetMaterial(ambient, diffuse, specular, shininess, em);
	if (true){//MVFXEnabled){
		nix::SetZ(alpha_zbuffer, alpha_zbuffer);
		if (transparency_mode == TransparencyModeColorKeyHard)
			nix::SetAlpha(AlphaColorKeyHard);
		else if (transparency_mode == TransparencyModeColorKeySmooth)
			nix::SetAlpha(AlphaColorKeySmooth);
		else if (transparency_mode == TransparencyModeFunctions){
			nix::SetAlpha(alpha_source, alpha_destination);
			//NixSetZ(false,false);
		}else if (transparency_mode == TransparencyModeFactor){
			nix::SetAlpha(alpha_factor);
			//NixSetZ(false,false);
		}
		nix::SetShader(material->shader);
	}
	Array<nix::Texture*> tex = textures;
	if (material->cube_map)
		tex.add(material->cube_map);
	nix::SetTextures(tex);
}

