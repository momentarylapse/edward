/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <lib/plugin/PluginManager.h>
#include <lib/xhui/_kaba_export.h>
#include <lib/syntaxhighlight/_kaba_export.h>
#include <Session.h>
#include <view/Mode.h>
#include <mode_model/data/DataModel.h>
#include <mode_world/data/DataWorld.h>
#include <y/plugins/PluginManager.h>
#include <y/world/components/Camera.h>
#include <y/world/components/Light.h>
#include <y/world/components/Link.h>
#include <y/world/Terrain.h>
#include <y/world/components/Skeleton.h>
#include <y/world/components/Animator.h>
#include <y/world/components/RigidBody.h>
#include <y/world/components/Collider.h>
#include <storage/Storage.h>

#include <lib/os/msg.h>
#include <lib/net/_kaba_export.h>
#include <lib/profiler/_kaba_export.h>
#include <lib/kaba/dynamic/dynamic.h>

namespace edward {

Session* cur_session = nullptr;


PluginManager::PluginManager(Session* s, const Path &dir) {
	session = s;
	directory = dir;
	init_edward();
}

PluginManager::~PluginManager() = default;

void PluginManager::execute(const Path& filename) {
	cur_session = session;
	//kaba::config.directory = "";
	try {
		auto s = session->kaba_ctx->_load_module_throw(filename, false);
		typedef void func_t();
		if (auto f = (func_t*)s->match_function("main", "void", {}))
			f();
	} catch(kaba::Exception& e) {
		session->error(e.message());
	}

	//kaba::DeleteAllScripts(true, true);
}


//hui::Window *GlobalMainWin = ed;

void PluginManager::init_edward() {
	msg_error("INIT EDWARD");
	session->kaba_ctx->register_package_init("codeeditor", directory.absolute().canonical() | "codeeditor", [] (kaba::IExporter* e) {
		msg_error("INIT CODEEDITOR");
		_export_package_syntaxhighlight_internal(e);
	});
	session->kaba_ctx->register_package_init("xhui", directory.absolute().canonical() | "xhui", &export_package_xhui);
	session->kaba_ctx->register_package_init("yengine", directory.absolute().canonical() | "yengine", &::PluginManager::export_kaba_package_yengine);

	link_plugins();
	find_plugins();
}

template<class T>
void link_component_x(shared<kaba::Module> mm, const string& name) {
	T::_class = mm->tree->create_new_class(name, kaba::MetaClass::NONE, nullptr, sizeof(T), 0, nullptr, {}, mm->tree->base_class, -1);
}

void PluginManager::load_project_stuff(const Path &dir) {
	//session->kaba_ctx->register_package_init("yengine", dir.absolute().canonical() | "Scripts/yengine", &::PluginManager::export_kaba_package_yengine);
	session->kaba_ctx->register_package_init("profiler", dir.absolute().canonical() | "Scripts/profiler", &export_package_profiler);
	session->kaba_ctx->register_package_init("net", dir.absolute().canonical() | "Scripts/net", &export_package_net);

	component_classes = enumerate_classes("ecs.Component");
	system_classes = enumerate_classes("ecs.System");

	auto link_component = [this] (const string& name) -> const kaba::Class* {
		for (auto* c : component_classes)
			if (c->name == name)
				return c;
		msg_error("component class not linkable: " + name);
		return nullptr;
	};

	Camera::_class = link_component("Camera");
	Light::_class = link_component("Light");
	Link::_class = link_component("Link");
	RigidBody::_class = link_component("RigidBody");
	MeshCollider::_class = link_component("MeshCollider");
	TerrainCollider::_class = link_component("TerrainCollider");
	Skeleton::_class = link_component("Skeleton");
	Animator::_class = link_component("Animator");
	ModelRef::_class = link_component("ModelRef");
	TerrainRef::_class = link_component("TerrainRef");
	TemplateRef::_class = link_component("TemplateRef");

	auto mm = session->kaba_ctx->create_empty_module("edward-internal");
	mm->_pointer_ref_counter = 999999;

	link_component_x<EdwardTag>(mm, ":EdwardTag:");
}

void PluginManager::find_plugins() {
	Path dir0 = (directory | "shader graph").absolute();
	auto list = os::fs::search(dir0, "*", "fd");
	for (auto &e: list) {
		Path dir = dir0 | e;
		if (os::fs::is_directory(dir)) {
			auto list2 = os::fs::search(dir, "*.kaba", "f");
			for (auto &e2: list2) {
				Plugin p;
				p.plugin_manager = this;
				p.filename = dir | e2;
				p.name = e2.str().replace(".kaba", "");
				p.category = e.str();
				p.type = PluginType::SHADER_NODE;
				plugins.add(p);
			}
		}
	}
}

const kaba::Class* PluginManager::find_class(const Path &filename, const string &parent) {
	//kaba::config.directory = "";
	auto s = session->kaba_ctx->_load_module_throw(filename, false);
	for (auto c: s->classes())
		if (c->is_derived_from_s(parent))
			return c;

	//throw Exception(format("no class derived from '%s' defined", parent));
	return nullptr;
}

void object_set_params(void* o, const kaba::Class* type, const Any& params) {
	for (const auto& e: type->elements)
		if (params.has(e.name)) {
			const auto v = params[e.name];
			void* p = (char*)o + e.offset;
			kaba::unwrap_any(v, p, e.type);
		}
}

SubMode* PluginManager::create_sub_mode(const string& id, Mode* parent, const Any& params) {
	if (auto c = session->plugin_manager->find_class(directory | "mode" | id + ".kaba", "edward.SubMode")) {
		for (auto ff: c->get_constructors())
			if (ff->num_params == 2) {
				auto m = (SubMode*)malloc(c->size);
				typedef void con_func(void *, void*);
				if (const auto f = reinterpret_cast<con_func*>(ff->address)) {
					f(m, parent);
					object_set_params(m, c, params);
					return m;
				}
			}
	}
	return nullptr;
}

Mode* PluginManager::create_root_mode(const string& id, DocumentSession* doc, const Any& params) {
	if (auto c = session->plugin_manager->find_class(directory | "mode" | id + ".kaba", "edward.Mode")) {
		for (auto ff: c->get_constructors())
			if (ff->num_params == 2) {
				auto m = (Mode*)malloc(c->size);
				typedef void con_func(void *, void*);
				if (const auto f = reinterpret_cast<con_func*>(ff->address)) {
					f(m, doc);
					object_set_params(m, c, params);
					return m;
				}
			}
	}
	return nullptr;
}

void *PluginManager::create_instance(const Path &filename, const string &parent) {
	//kaba::config.directory = "";
	if (auto c = find_class(filename, parent))
		return c->create_instance();

	throw Exception(format("no class derived from '%s' defined", parent));
	return nullptr;
}

void *PluginManager::Plugin::create_instance(const string &parent) const {
	return plugin_manager->create_instance(filename, parent);
}

Array<plugin::InstanceDataVariable> load_variables(const kaba::Class* c, const void* instance = nullptr) {
	Array<plugin::InstanceDataVariable> variables;
	for (auto cc: weak(c->constants))
		if (cc->type.get() == kaba::common_types.string and cc->name == "PARAMETERS") {
			auto params = cc->as_string().explode(",");
			for (auto& v: c->elements)
				if (sa_contains(params, v.name)) {
					if (instance)
						variables.add({v.name, plugin::whatever_to_any((const char*)instance + v.offset, v.type)});
					else
						variables.add({v.name, {}});
				}
		}
	return variables;
}

Array<const kaba::Class*> PluginManager::enumerate_classes(const string& full_base_class) {
	string base_class = full_base_class.explode(".").back();
	Array<const kaba::Class*> r;

	auto find_classes = [&] (const Path& dir, bool is_edward_dir) {
		const auto files = os::fs::search(dir, "*.kaba", "rf");
		for (auto &f: files) {
			if (f.is_in("yengine") != is_edward_dir)
				continue;
			try {
				auto s = session->kaba_ctx->_load_module_throw(dir | f, false);//, true);
				for (auto c: s->classes())
					if (c->is_derived_from_s(full_base_class) and c->name != base_class)
						r.add(c);
			} catch (Exception &e) {
				msg_error(e.message());
			}
		}
	};

	find_classes(directory, true);
	find_classes(session->storage->root_dir_kind[FD_SCRIPT], false);
	return r;
}

plugin::InstanceData PluginManager::describe_class(const kaba::Class* type, const void* instance) {
	auto variables = load_variables(type, instance);
	return {type->name, type->owner->module->filename.relative_to(session->storage->root_dir_kind[FD_SCRIPT]), variables};
}


void PluginManager::update_class(plugin::InstanceData& _c) {
	try {
		auto s = session->kaba_ctx->_load_module_throw(session->storage->root_dir_kind[FD_SCRIPT] | _c.filename, true);
		for (auto c: s->classes())
			if (c->name == _c.class_name) {
				auto variables = load_variables(c);
				for (const auto& v: variables) {
					bool has = false;
					for (const auto& x: _c.variables)
						if (x.name == v.name)
							has = true;
					if (!has)
						_c.variables.add(v);
				}
			}
	} catch (Exception &e) {
		msg_error(e.message());
	}
}

const kaba::Class *PluginManager::get_class(const plugin::InstanceData &desc) const {
	for (const auto c: component_classes)
		if (c->name == desc.class_name)
			return c;
	for (const auto c: system_classes)
		if (c->name == desc.class_name)
			return c;
	return nullptr;
}
}
