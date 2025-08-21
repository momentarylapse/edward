#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/pointer.h>
#include <lib/os/path.h>
#include <lib/image/color.h>
#include <lib/ygraphics/graphics-fwd.h>

#include "lib/any/any.h"

#define MATERIAL_MAX_TEXTURES		8

class Path;
class Model;

namespace yrenderer {


// types of transparency
enum class TransparencyMode {
	NONE = 0,
	FUNCTIONS = 1,
	COLOR_KEY_HARD = 2,
	COLOR_KEY_SMOOTH = 3,
	FACTOR = 4,
	MIX = 5,
	DEFAULT = -1,
};

// types of reflection
enum class ReflectionMode {
	NONE,
	METAL,
	MIRROR,
	CUBE_MAP_STATIC,
	CUBE_MAP_DYNAMIC
};

/*enum class ShaderVariant {
	DEFAULT,
	ANIMATED,
	INSTANCED,
	LINES,
	POINTS,
	_NUM
};*/

enum class RenderPathType;

struct Context;
class ShaderManager;

// visual and physical properties
class Material {
public:
	Context* ctx;

	shared_array<ygfx::Texture> textures;

	Any shader_data;

	// light
	color albedo, emission;
	float roughness, metal;

	bool cast_shadow;

	struct RenderPassData {
		TransparencyMode mode = TransparencyMode::NONE;
		ygfx::Alpha source, destination;
		float factor = 1;
		bool z_buffer = true;
		bool z_test = true;
		ygfx::CullMode cull_mode = (ygfx::CullMode)0;
		Path shader_path;
	};
	int num_passes = 1;
	RenderPassData pass0;

	struct ExtendedData {
		RenderPassData pass[4];
	};
	owned<ExtendedData> extended;

	struct Reflection {
		ReflectionMode mode;
		float density;
		//CubeMap *cube_map;
		ygfx::Texture *cube_map;
		int cube_map_size;
	} reflection;

	struct Friction {
		float jump, _static, sliding, rolling;
	} friction;

	explicit Material(Context* ctx);
	xfer<Material> copy();

	bool is_transparent() const;
	const RenderPassData& pass(int k) const;
	RenderPassData& pass(int k);
};

struct ShaderCache {
	shared<ygfx::Shader> shader[2]; // * #(render paths)
	void _prepare_shader(RenderPathType render_path_type, const Material& material, const string& vertex_module, const string& geometry_module);
	void _prepare_shader_multi_pass(RenderPathType render_path_type, const Material& material, const string& vertex_module, const string& geometry_module, int k);
	ygfx::Shader *get_shader(RenderPathType render_path_type);
};

}


