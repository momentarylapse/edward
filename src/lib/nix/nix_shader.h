/*----------------------------------------------------------------------------*\
| Nix shader                                                                   |
| -> shader files                                                              |
|                                                                              |
| last update: 2010.03.11 (c) by MichiSoft TM                                  |
\*----------------------------------------------------------------------------*/

#if HAS_LIB_GL

#ifndef _NIX_SHADER_EXISTS_
#define _NIX_SHADER_EXISTS_

#include "../base/pointer.h"

namespace nix{

class Buffer {
public:
	enum class Type {
		NONE,
		UNIFORM,
		SSBO
	} type;
	unsigned int buffer;

	Buffer();
	~Buffer();

	void __delete__();
	void update(void *data, int size);
	void update_array(const DynamicArray &a);

	void read(void *data, int size);
	void read_array(DynamicArray &a);
};

class UniformBuffer : public Buffer {
public:
	UniformBuffer();
	void __init__();
};

class ShaderStorageBuffer : public Buffer {
public:
	ShaderStorageBuffer();
	void __init__();
};

void bind_buffer(Buffer *buf, int binding);

class Shader : public Sharable<Empty> {
public:
	Path filename;
	int program;
	Shader();
	~Shader();
	void _cdecl set_float_l(int location, float f);
	void _cdecl set_int_l(int location, int i);
	void _cdecl set_floats_l(int location, const float *data, int num);
	void _cdecl set_matrix_l(int location, const matrix &m);
	void _cdecl set_color_l(int location, const color &c);
	int _cdecl get_location(const string &name);

	void _cdecl set_float(const string &name, float f);
	void _cdecl set_int(const string &name, int i);
	void _cdecl set_floats(const string &name, const float *data, int num);
	void _cdecl set_matrix(const string &name, const matrix &m);
	void _cdecl set_color(const string &name, const color &c);

	bool _cdecl link_uniform_block(const string &name, int binding);
	
	void _cdecl dispatch(int nx, int ny, int nz);

	void find_locations();
	void set_default_data();

	enum {
		LOCATION_MATRIX_MVP,
		LOCATION_MATRIX_M,
		LOCATION_MATRIX_V,
		LOCATION_MATRIX_P,
		LOCATION_TEX,
		LOCATION_TEX_CUBE = LOCATION_TEX + NIX_MAX_TEXTURELEVELS,
		LOCATION_MATERIAL_ALBEDO,
		LOCATION_MATERIAL_ROUGHNESS,
		LOCATION_MATERIAL_METAL,
		LOCATION_MATERIAL_EMISSION,
		NUM_LOCATIONS
	};

	int location[NUM_LOCATIONS];


	static Shader* _cdecl load(const Path &filename);
	static Shader* _cdecl create(const string &source);
	void _cdecl update(const string &source);


	static Shader *default_2d;
	static Shader *default_3d;
	static Shader *default_load;
	static Shader *_current_;
};


void init_shaders();
void _cdecl delete_all_shaders();
void _cdecl set_shader(Shader *s);


};

#endif

#endif
