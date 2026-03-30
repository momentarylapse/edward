#include "ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/os/filesystem.h>
#include <EngineData.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/Context.h>
#include <world/ModelManager.h>
#include <world/Terrain.h>

#include "lib/os/file.h"

#ifdef USING_VULKAN
namespace vulkan {
	extern string overwrite_bindings;
	extern int overwrite_push_size;
}
#endif

class TemplateManager {
public:
	base::map<Path, Template*> templates;

	Path full_path(const Path& _filename) const {
		auto filename = _filename.is_absolute() ? _filename : engine.object_dir | _filename;
		if (filename.extension() != "template")
			return filename.with(".template");
		return filename;
	}

	// relative filename!
	Template* load(const Path& _filename) {
		auto filename = full_path(_filename);
		for (auto&& [f, t]: templates)
			if (f == filename)
				return t;
		if (!os::fs::exists(filename))
			return nullptr;
		auto t = new Template;
		templates.set(filename, t);
		ScriptInstanceData* current = nullptr;
		const auto s = os::fs::read_text(filename);
		for (const auto& l: s.explode("\n")) {
			if (l.num == 0)
				continue;
			if (l[0] == '\t') {
				if (current) {
					const auto xx = l.sub_ref(1).explode("=");
					if (xx.num >= 2) {
						current->variables.add({xx[0], Any::parse(xx[1])});
					}
				}
			} else {
				int p = l.find(" ");
				if (p >= 0) {
					t->components.add({l.sub_ref(0, p), l.sub_ref(p + 1)});
				} else {
					t->components.add({l});
				}
				current = &t->components.back();
			}
		}
		return t;
	}

	void save(const Template* t, const Path& filename) {
		string o;
		for (const auto& c: t->components) {
			if (c.filename and !c.filename.is_in("y"))
				o += format("%s %s\n", c.class_name, c.filename);
			else
				o += c.class_name + "\n";
			for (const auto& v: c.variables)
				o += format("\t%s=%s\n", v.name, str(v.value));
		}
		os::fs::write_text(full_path(filename), o);
	}

	Path get_filename(const Template* t) const {
		for (auto&& [f, _t]: templates)
			if (_t == t)
				return f.relative_to(engine.object_dir);
		return "";
	}

	void clear() {
		for (auto&& [f, t]: templates)
			delete t;
		templates.clear();
	}
};

class TerrainManager {
public:
	owned_array<Terrain> terrains;

	Terrain* load_lazy(const Path& filename) {
		if (!filename)
			return nullptr;
		for (auto t: weak(terrains))
			if (t->filename == filename)
				return t;
		auto t = new Terrain();
		t->filename = filename;
		terrains.add(t);
		return t;
	}

	Terrain* load(ResourceManager* rm, const Path& filename) {
		auto t = load_lazy(filename);
		if (t and t->filename and t->height.num == 0)
			t->reload(rm, true);
		return t;
	}

	Path get_filename(const Terrain *t) const {
		for (auto _t: weak(terrains))
			if (_t == t)
				return _t->filename;
		return "";
	}

	void clear() {
		terrains.clear();
	}
};


ResourceManager::ResourceManager(yrenderer::Context *_ctx, const Path &texture_dir, const Path &material_dir, const Path &shader_dir) {
	ctx = _ctx;
	texture_manager = new yrenderer::TextureManager(ctx ? ctx->context : nullptr, texture_dir);
	material_manager = new yrenderer::MaterialManager(texture_manager, material_dir);
	model_manager = new ModelManager(this, material_manager);
	shader_manager = new yrenderer::ShaderManager(ctx ? ctx->context : nullptr, shader_dir);
	shader_manager->ignore_missing_files = engine.ignore_missing_files;
	template_manager = new TemplateManager();
	terrain_manager = new TerrainManager();
}

yrenderer::Material* ResourceManager::load_material(const Path& filename) {
	return material_manager->load(filename);
}

xfer<Model> ResourceManager::load_model_copy(const Path& filename) {
	return model_manager->load_copy(filename);
}

Model* ResourceManager::load_model(const Path& filename) {
	return model_manager->load(filename);
}

Terrain* ResourceManager::load_terrain_lazy(const Path& filename) {
	return terrain_manager->load_lazy(filename);
}

Terrain* ResourceManager::load_terrain(const Path& filename) {
	return terrain_manager->load(this, filename);
}

Template* ResourceManager::load_template(const Path &filename) {
	return template_manager->load(filename);
}

void ResourceManager::save_template(const Template *t, const Path &filename) {
	template_manager->save(t, filename);
}


shared<ygfx::Texture> ResourceManager::load_texture(const Path& filename) {
	return texture_manager->load_texture(filename);
}

Path ResourceManager::filename(const Model *m) const {
	return model_manager->get_filename(m);
}

Path ResourceManager::filename(const Terrain* t) const {
	return terrain_manager->get_filename(t);
}

Path ResourceManager::filename(const yrenderer::Material* m) const {
	return material_manager->get_filename(m);
}

Path ResourceManager::filename(const ygfx::Texture* t) const {
	return texture_manager->get_filename(t);
}

Path ResourceManager::filename(const Template* t) const {
	return template_manager->get_filename(t);
}

void ResourceManager::clear() {
	shader_manager->clear();
	texture_manager->clear();
	material_manager->reset();
	template_manager->clear();
	terrain_manager->clear();
}


