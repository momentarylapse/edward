//
// Created by michi on 7/30/25.
//

#include "ShaderManager.h"
#include "graphics-impl.h"
#include "Context.h"
#include <lib/os/filesystem.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <lib/os/app.h>
#include <lib/image/image.h>

static const string DEFAULT_BINDINGS = "[[sampler,sampler,sampler,sampler,sampler,sampler,sampler,sampler,ubo,ubo,ubo,ubo,storage-buffer]]";
static const string DEFAULT_VERSION = "460";
static constexpr int DEFAULT_PUSH_SIZE = 96;

[[maybe_unused]] const int TYPE_LAYOUT = -41;
[[maybe_unused]] const int TYPE_MODULE = -42;

#ifdef USING_VULKAN
#else
namespace nix {
	extern string overwrite_shader_version;
}
#endif

namespace ygfx {

struct ShaderMetaData {
	string version, name, bindings;
	Array<string> extensions;
	int push_size = 0;
};

struct ShaderModule {
	Path filename;
	ShaderMetaData meta;
	string source;
};

Path guess_absolute_path(const Path &filename, const Array<Path>& dirs) {
	if (filename.is_empty())
		return Path::EMPTY;

	if (filename.is_absolute())
		return filename;

	for (auto &d: dirs)
		if (os::fs::exists(d | filename))
			return d | filename;

	return Path::EMPTY;
}


ShaderManager::ShaderManager(Context* _ctx, const Array<Path>& _shader_dirs) {
	ctx = _ctx;
	shader_dirs = _shader_dirs;
	default_shader = "default.shader";
}

ShaderManager::~ShaderManager() = default;

void ShaderManager::inherit_modules_from(const ShaderManager& shader_manager) {
	shader_modules = shader_manager.shader_modules;
}

void ShaderManager::add_directory(const Path& dir) {
	shader_dirs.add(dir);
}


base::result<shared<Shader>> ShaderManager::__load_shader(const Path& path, const string& overwrite_version, const string& overwrite_bindings, int overwrite_push_size) {
//#ifdef USING_VULKAN
	if (!path)
		return shared<Shader>(nullptr);
	//if (verbosity >= 1)
	msg_write(format("load shader %s", path));

	//	if (!os::fs::exists(filename.with(".compiled")))
	RESULT_PROPAGATE_ERROR(source, os::fs::read_text2(path), x1);
	RESULT_PROPAGATE_ERROR(s, __create_shader(source, overwrite_version, overwrite_bindings, overwrite_push_size), x2);

	return s;


	//msg_write("loading shader: " + str(path));
	/*return Shader::load(path).transform<shared<Shader>>([] (Shader* s) {
		return shared{s};
	});*/

/*#else
	nix::overwrite_shader_version = overwrite_version;
	try {
		return shared{ctx->ctx->load_shader(path)};
	} catch (Exception& e) {
		return base::Error(e.message());
	}
#endif*/
}

	struct ShaderSourcePart {
		string type;
		string source;
	};

#ifdef USING_VULKAN
#define STAGE(a, b) (int)a
#else
#define STAGE(a, b) b
#endif

	Array<ShaderSourcePart> get_shader_parts(const string &source) {
		Array<ShaderSourcePart> parts;
		bool has_vertex = false;
		bool has_fragment = false;
		int pos = 0;
		while (pos < source.num - 5) {
			int pos0 = source.find("<", pos);
			if (pos0 < 0)
				break;
			pos = pos0 + 1;
			int pos1 = source.find(">", pos0);
			if (pos1 < 0)
				break;

			string tag = source.sub(pos0 + 1, pos1);
			if ((tag.num > 64) or (tag.find("<") >= 0))
				continue;

			int pos2 = source.find("</" + tag + ">", pos1 + 1);
			if (pos2 < 0)
				continue;
			ShaderSourcePart p;
			p.source = source.sub(pos1 + 1, pos2);
			p.type = tag;
			pos = pos2 + tag.num + 3;
			if (tag == "VertexShader") {
				//p.type = STAGE(VK_SHADER_STAGE_VERTEX_BIT, GLSL;
				has_vertex = true;
			} else if (tag == "FragmentShader") {
				//p.type = VK_SHADER_STAGE_FRAGMENT_BIT;
				has_fragment = true;
			} /*else if (tag == "ComputeShader") {
				p.type = VK_SHADER_STAGE_COMPUTE_BIT;
			} else if (tag == "TessellationControlShader") {
				p.type = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			} else if (tag == "TessellationEvaluationShader") {
				p.type = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			} else if (tag == "GeometryShader") {
				p.type = VK_SHADER_STAGE_GEOMETRY_BIT;
			} else if (tag == "RayGenShader") {
				p.type = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			} else if (tag == "RayClosestHitShader") {
				p.type = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			} else if (tag == "RayAnyHitShader") {
				p.type = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			} else if (tag == "RayMissShader") {
				p.type = VK_SHADER_STAGE_MISS_BIT_KHR;
			} else if (tag == "TaskShader") {
				p.type = VK_SHADER_STAGE_TASK_BIT_EXT;
			} else if (tag == "MeshShader") {
				p.type = VK_SHADER_STAGE_MESH_BIT_EXT;
			} else if (tag == "Module") {
				p.type = TYPE_MODULE;
			} else if (tag == "Layout") {
				p.type = TYPE_LAYOUT;
			} else {
				msg_error("unknown shader tag: '" + tag + "'");
				continue;
			}*/
			parts.add(p);
		}
		/*if (has_fragment and !has_vertex) {
			if (verbosity >= 1)
				msg_write(" ...auto import " + vertex_module_default);
			parts.add({VK_SHADER_STAGE_VERTEX_BIT, format("#import %s\n", vertex_module_default)});
		}*/
		return parts;
	}

	string get_inside_of_tag(const string &source, const string &tag) {
		string r;
		int pos0 = source.find("<" + tag + ">");
		if (pos0 < 0)
			return "";
		pos0 += tag.num + 2;
		int pos1 = source.find("</" + tag + ">", pos0);
		if (pos1 < 0)
			return "";
		return source.sub(pos0, pos1);
	}

	base::result<string> ShaderManager::expand_shader_source(const string &source, const ShaderMetaData &meta) {
		string r = source;
		while (true) {
			int p = r.find("#import", 0);
			if (p < 0)
				break;
			int p2 = r.find("\n", p);
			string imp = r.sub(p + 7, p2).replace(" ", "");
			//msg_error("import '" + imp + "'");

			bool found = false;
			for (const auto &m: shader_modules)
				if (m.meta.name == imp) {
					//msg_error("FOUND " + imp);
					r = r.head(p) + "\n// << + " + imp + "\n" + m.source + "\n// >> " + imp + "\n" + r.sub(p2);
					found = true;
					break;
				}
			if (!found)
				return base::Error{format("shader import '%s' not found xxx", imp)};
		}

		string intro;
		if (meta.version != "")
			intro += "#version " + meta.version + "\n";
		for (auto &e: meta.extensions)
			intro += "#extension " + e + " : require\n";
		if (r.find("GL_ARB_separate_shader_objects", 0) < 0)
			intro += "#extension GL_ARB_separate_shader_objects : enable\n";
		/*if (false) {
			msg_write("\n\n======================================");
			msg_write(with_line_numbers(intro + r));
			msg_write("======================================\n\n");
		}*/
		return intro + r;
	}

	ShaderMetaData parse_meta(const string& source, const ShaderMetaData& overwrite) {
		ShaderMetaData m;
		for (auto &x: source.explode("\n")) {
			auto y = x.explode("=");
			if (y.num == 2) {
				string k = y[0].trim();
				string v = y[1].trim();
				if (k == "name") {
					m.name = v;
				} else if (k == "version") {
					m.version = v;
				} else if ((k == "binding") or (k == "bindings")) {
					m.bindings = v;
				} else if (k == "pushsize") {
					m.push_size = v._int();
				} else if (k == "input") {
				} else if (k == "topology") {
				} else if (k == "extensions") {
					m.extensions = v.explode(",");
				} else {
					msg_error("unhandled shader meta: " + x);
				}
			}
		}
		if (overwrite.version != "")
			m.version = overwrite.version;
		if (overwrite.bindings != "")
			m.bindings = overwrite.bindings;
		if (overwrite.push_size >= 0)
			m.push_size = overwrite.push_size;
		return m;
	}

base::result<shared<Shader>> ShaderManager::__create_shader(const string& source, const string& overwrite_version, const string& overwrite_bindings, int overwrite_push_size) {
	auto parts = get_shader_parts(source);

	if (parts.num == 0)
		return base::Error{"no shader tags found (<VertexShader>...</VertexShader> or <FragmentShader>...</FragmentShader>)"};

#ifdef USING_VULKAN
	shared s = new Shader();
#else
	shared s = new Shader(ctx->ctx);
#endif

	ShaderMetaData meta;
	for (const auto& p: parts) {
		if (p.type == "Layout") {
			meta = parse_meta(p.source, {overwrite_version, "", overwrite_bindings, {}, overwrite_push_size});
		} else {
			RESULT_PROPAGATE_ERROR(a, expand_shader_source(p.source, meta), x2);
			RESULT_PROPAGATE_ERROR_VOID(s->add_module(p.type, a));
		}
	}
#ifdef USING_VULKAN
	s->set_meta(meta.bindings, meta.push_size);
#else
	RESULT_PROPAGATE_ERROR_VOID(s->build());
#endif
	return s;
}

base::result<shared<Shader>> ShaderManager::load_shader(const Path& filename) {
	//if (!filename)
	//	TODO default shader?
	//	return __load_shader("");

	Path fn = guess_absolute_path(filename, shader_dirs);
	if (!fn) {
		if (ignore_missing_files) {
			msg_error("missing shader: " + str(filename));
			return __load_shader("", "", "", -1);
		}
		return base::Error{"missing shader: " + str(filename)};
		//fn = shader_dir | filename;
	}

	for (auto&& [key, s]: shader_map)
		if (key == fn) {
#ifdef USING_VULKAN
			return shared{s};
#else
			return (s->program >= 0) ? base::result{shared{s}} : base::Error{"..."};
#endif
		}

	RESULT_PROPAGATE_ERROR(s, __load_shader(fn, "", "", -1), x1);

	shaders.add(s);
	shader_map.set(fn, s.get());
	return s;
}

string ShaderManager::expand_vertex_shader_source(const string &source, const string &variant) {
	if (source.find("<VertexShader>") >= 0)
		return source;
	//msg_write("INJECTING " + variant);
	return source + format("\n<VertexShader>\n#import vertex-%s\n</VertexShader>", variant);
}

string ShaderManager::expand_fragment_shader_source(const string &source, const string &render_path) {
	if (render_path.num > 0)
		return source.replace("#import surface", "#import surface-" + render_path);
	return source;
}

string ShaderManager::expand_geometry_shader_source(const string &source, const string &variant) {
	if (source.find("<GeometryShader>") >= 0)
		return source;
	return source + format("\n<GeometryShader>\n#import geometry-%s\n</GeometryShader>", variant);
}

string ShaderManager::expand_tessellation_control_shader_source(const string &source, const string &variant) {
	if (source.find("<TessellationControlShader>") >= 0)
		return source;
	return source + format("\n<TessellationControlShader>\n#import tessellation-control-%s\n</TessellationControlShader>", variant);
}

string ShaderManager::expand_tessellation_evaluation_shader_source(const string &source, const string &variant) {
	if (source.find("<TessellationEvaluationShader>") >= 0)
		return source;
	return source + format("\n<TessellationEvaluationShader>\n#import tessellation-evaluation-%s\n</TessellationEvaluationShader>", variant);
}

base::result<shared<Shader>> ShaderManager::load_surface_shader(const Path& _filename, const string& render_path, const string& vertex_module, const string& geometry_module, const string& tessellation_module) {
	//msg_write("load_surface_shader: " + str(_filename) + "  " + render_path + "  " + vertex_module + "  " + geometry_module);
	//select_default_vertex_module("vertex-" + variant);
	//return load_shader(filename);
	auto filename = _filename;
	if (!filename)
		filename = default_shader;



	if (!filename)
		return dummy_surface_shader();

	Path fn = guess_absolute_path(filename, shader_dirs);
	if (fn.is_empty()) {
		if (ignore_missing_files) {
			msg_error("missing shader: " + str(filename));
			return dummy_surface_shader();
		}
		return base::Error{"missing shader: " + str(filename)};
		//fn = shader_dir | filename;
	}

	Path fnx = fn.with(":" + render_path +  ":" + vertex_module + ":" + geometry_module);
	if (tessellation_module != "")
		fnx = fnx.with(":" + tessellation_module);
	for (auto&& [key, s]: shader_map)
		if (key == fnx) {
#ifdef USING_VULKAN
			return shared{s};
#else
			return (s->program >= 0) ? base::result{shared{s}} : base::Error{"..."};
#endif
		}


	msg_write("loading shader: " + str(fnx));

	string source = expand_vertex_shader_source(os::fs::read_text(fn), vertex_module);
	if (geometry_module != "")
		source = expand_geometry_shader_source(source, geometry_module);
	if (tessellation_module != "") {
		source = expand_tessellation_control_shader_source(source, tessellation_module);
		source = expand_tessellation_evaluation_shader_source(source, tessellation_module);
	}
	source = expand_fragment_shader_source(source, render_path);

	RESULT_PROPAGATE_ERROR(shader, __create_shader(source, DEFAULT_VERSION, DEFAULT_BINDINGS, DEFAULT_PUSH_SIZE), x1);
	//auto s = Shader::load(fn);

	shaders.add(shader);
	shader_map.set(fnx, shader.get());
	return shader;
}

shared<Shader> ShaderManager::dummy_surface_shader() {
	if (!dummy_shader)
		dummy_shader = REQUIRED(__create_shader(expand_vertex_shader_source("<Layout>\n</Layout>\n<FragmentShader>\n#import basic-data\nvoid main() { out_color = vec4(1,0,0,1); }\n</FragmentShader>", "default"),
			DEFAULT_VERSION, DEFAULT_BINDINGS, DEFAULT_PUSH_SIZE));
	return dummy_shader;
}

base::result<shared<Shader>> ShaderManager::create_shader(const string &source) {
	return __create_shader(source, "", "", -1);
}

base::result_void ShaderManager::load_shader_module(const Path& path) {
	Path fn = guess_absolute_path(path, shader_dirs);
	if (!fn)
		return base::Error{format("shader module not found: %s", path)};
	for (const auto& m: shader_modules)
		if (m.filename == fn)
			return base::result_success();

	RESULT_PROPAGATE_ERROR(source, os::fs::read_text2(fn), x1);
	return create_shader_module(source, fn);
}

base::result<ShaderModule> _create_shader_module(const string& source, const Path& filename) {
	auto parts = get_shader_parts(source);

	ShaderMetaData meta;
	for (const auto& p: parts) {
		if (p.type == "Layout") {
			meta = parse_meta(p.source, {});
		} else if (p.type == "Module") {
			ShaderModule m;
			m.filename = filename;
			m.source = p.source;
			m.meta = meta;
			return m;
		}
	}
	return base::Error("no <Module> tag found");
}

base::result_void ShaderManager::create_shader_module(const string& source, const Path& filename) {
	RESULT_PROPAGATE_ERROR(m, _create_shader_module(source, filename), x1);
	shader_modules.add(m);
	return base::result_success();
}

void ShaderManager::reload_modules() {
	for (auto& m: shader_modules)
		if (auto source = os::fs::read_text2(m.filename))
			if (auto mm = _create_shader_module(*source, m.filename))
				m = *mm;
}


void ShaderManager::clear() {
	shaders.clear();
	shader_map.clear();
	dummy_shader = nullptr;
}

}
