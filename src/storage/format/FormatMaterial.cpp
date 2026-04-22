/*
 * FormatMaterial.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatMaterial.h"
#include "../../Session.h"
//#include "../../data/material/ShaderGraph.h"
#include "../../lib/nix/nix.h"
#include "../../lib/os/config.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../lib/os/msg.h"
#include "helper/ResourceManager.h"
#include "lib/yrenderer/MaterialManager.h"
#include "lib/yrenderer/TextureManager.h"

namespace yrenderer {
	color any2color(const Any &a);
}

namespace yrenderer {
	ygfx::Alpha parse_alpha(const string& s);
}

FormatMaterial::FormatMaterial(Session *s) : TypedFormat<DataMaterial>(s, FD_MATERIAL, "material", "Material", Flag::CANONICAL_READ_WRITE) {
}

Array<Path> str_arr_to_paths(const Array<string> &s) {
	Array<Path> r;
	for (auto &x: s)
		r.add(x);
	return r;
}

void FormatMaterial::load_current(const Path &filename, DataMaterial *data) {
	session->resource_manager->material_manager->_load_from_file(&data->material, filename);
}

void FormatMaterial::load_legacy(LegacyFile &lf, DataMaterial *data) {
	auto f = lf.f;
	if ((lf.ffv == 3) or (lf.ffv == 4)) {
		if (lf.ffv >= 4) {
			f->read_comment();
			int n = f->read_int();
			data->material.textures.resize(n);
			for (int i=0;i<n;i++)
				data->material.textures[i] = session->resource_manager->load_texture(f->read_str());
		}
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		read_color_argb(f, data->material.albedo);
		data->material.roughness = 0.5f;
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		/*shininess = */ f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		data->material.pass0.z_write = f->read_bool();
		data->material.pass0.z_test = true;
		// Appearance
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_bool();
		// Reflection
		f->read_comment();
		f->read_int();
		f->read_int();
		f->read_int();
		for (int i=0;i<6;i++)
			f->read_str();
		// ShaderFile
		f->read_comment();
		data->material.pass0.shader_path = f->read_str();
		// Physics
		f->read_comment();
		data->material.friction.jump = (float)f->read_int() * 0.001f;
		data->material.friction._static = (float)f->read_int() * 0.001f;
		data->material.friction.sliding = (float)f->read_int() * 0.001f;
		data->material.friction.rolling = (float)f->read_int() * 0.001f;
		/*data->material.vmin_jump =*/ (float)f->read_int() * 0.001f;
		/*data->material.vmin_sliding =*/ (float)f->read_int() * 0.001f;

		//AlphaZBuffer=(TransparencyMode!=TransparencyMode::FUNCTIONS)and(TransparencyMode!=TransparencyMode::FACTOR);
	}else if (lf.ffv==2){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->material.roughness = 0.5f;
		read_color_argb(f, data->material.albedo);
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		[[maybe_unused]] auto shininess = (float)f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->material.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		bool Mirror = f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		string sf = f->read_str();
		if (sf.num > 0)
			data->material.pass0.shader_path = sf + ".fx.glsl";
		// Physics
		f->read_comment();
		data->material.friction.jump = (float)f->read_int() * 0.001f;
		data->material.friction._static = (float)f->read_int() * 0.001f;
		data->material.friction.sliding = (float)f->read_int() * 0.001f;
		data->material.friction.rolling = (float)f->read_int() * 0.001f;
		/*data->material.vmin_jump =*/ (float)f->read_int() * 0.001f;
		/*data->material.vmin_sliding =*/ (float)f->read_int() * 0.001f;

		data->material.pass0.z_write = (data->material.pass0.mode != yrenderer::TransparencyMode::FUNCTIONS) and (data->material.pass0.mode != yrenderer::TransparencyMode::FACTOR);
		data->material.pass0.z_test = true;
	}else if (lf.ffv==1){
		// Colors
		f->read_comment();
		color cc;
		read_color_argb(f, cc);
		data->material.roughness = 0.5f;
		read_color_argb(f, data->material.albedo);
		read_color_argb(f, cc);
		data->material.metal = 0.1f;
		[[maybe_unused]] auto shininess = (float)f->read_int();
		read_color_argb(f, data->material.emission);
		// Transparency
		f->read_comment();
		data->material.pass0.mode = (yrenderer::TransparencyMode)f->read_int();
		data->material.pass0.factor = (float)f->read_int() * 0.01f;
		data->material.pass0.source = (ygfx::Alpha)f->read_int();
		data->material.pass0.destination = (ygfx::Alpha)f->read_int();
		// Appearance
		f->read_comment();
		int MetalDensity = f->read_int();
		if (MetalDensity > 0)
			data->material.metal = (float)MetalDensity * 0.01f;
		f->read_int();
		f->read_int();
		f->read_bool();
		f->read_bool();
		// ShaderFile
		f->read_comment();
		Path sf = f->read_str();
		if (!sf.is_empty())
		data->material.pass0.shader_path = sf.with(".fx.glsl");

		data->material.pass0.z_write = (data->material.pass0.mode != yrenderer::TransparencyMode::FUNCTIONS) and (data->material.pass0.mode != yrenderer::TransparencyMode::FACTOR);
		data->material.pass0.z_test = true;
	}else{
		//throw FormatError(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename, ffv, 1, 4));
	}
}

void FormatMaterial::_load(const Path &filename, DataMaterial *data, bool deep) {

	int ffv;
	data->reset();

	Stream *f = nullptr;


	auto lf = file_get_legacy_header(filename);
	//data->file_time = f->mtime().time;

	if (lf)
		load_legacy(*lf, data);
	else
		load_current(filename, data);

	/*if (ffv<0){
		throw FormatError(_("File format unreadable!"));*/

#if 0
	if (deep) {
		for (auto &p: data->material.passes)
			p.shader.load_from_file(data->doc);
	}
#endif

	delete f;
}




void FormatMaterial::_save(const Path &filename, DataMaterial *data) {
	session->resource_manager->material_manager->_write_to_file(&data->material, filename);
}
