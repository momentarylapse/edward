/*
 * FormatWorld.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatWorld.h"
#include "../../mode_world/data/DataWorld.h"
#include "../../mode_world/data/WorldLink.h"
#include "../../mode_world/data/WorldObject.h"
#include "../../mode_world/data/WorldTerrain.h"
//#include "../../EdwardWindow.h"
#include "../../Session.h"
//#include "../../stuff/Progress.h"
#include <lib/base/iter.h>
#include <y/world/Model.h>
#include <y/world/World.h>
#include <y/world/Camera.h>
#include <y/world/Light.h>
#include <y/y/EngineData.h>
#include <y/world/ModelManager.h>
#include <y/helper/ResourceManager.h>
#include <y/world/components/Skeleton.h>
#include <y/world/components/Collider.h>
#include <y/world/components/SolidBody.h>
#include <y/world/components/Animator.h>
#include <y/world/Terrain.h>
#include "../../lib/os/filesystem.h"
#include "../../lib/os/msg.h"
#include "../../lib/doc/xml.h"
#include <y/EntityManager.h>
#include <stuff/PluginManager.h>
#include <meta.h>
#include <lib/kaba/kaba.h>


static string _(const string &s) { return s; }

FormatWorld::FormatWorld(Session *s) : TypedFormat<DataWorld>(s, FD_WORLD, "world", _("World"), Flag::CANONICAL_READ_WRITE) {
}


static string v2s(const vec3 &v) {
	return format("%.3f %.3f %.3f", v.x, v.y, v.z);
}

static vec3 s2v(const string &s) {
	auto x = s.explode(" ");
	if (x.num >= 3)
		return vec3(x[0]._float(), x[1]._float(), x[2]._float());
	return v_0;
}

// RGBA
static string c2s(const color &c) {
	return format("%.3f %.3f %.3f %.3f", c.r, c.g, c.b, c.a);
}

static color s2c(const string &s) {
	auto x = s.explode(" ");
	if (x.num >= 4)
		return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
	if (x.num == 3)
		return color(1, x[0]._float(), x[1]._float(), x[2]._float());
	return White;
}

// script vars
string vars2str(const Array<ScriptInstanceDataVariable>& vars) {
	string s;
	for (auto &v: vars) {
		if (v.value == "")
			continue;
		if (s != "")
			s += ", ";
		s += v.name + ":" + v.value;
	}
	return s;
}

Array<ScriptInstanceDataVariable> str2vars(const string& s) {
	Array<ScriptInstanceDataVariable> vars;
	for (auto &x: s.explode(",")) {
		auto xx = x.explode(":");
		if (xx.num != 2)
			continue;
		ScriptInstanceDataVariable v;
		v.name = xx[0].trim();
		v.value = xx[1].trim();
		vars.add(v);
	}
	return vars;
}

void FormatWorld::_load(const Path &filename, DataWorld *data, bool deep) {
	data->reset();

	if (auto lf = file_get_legacy_header(filename))
		_load_old(*lf, data, deep);
	else
		_load_xml(filename, data, deep);



	if (deep) {
		try {
			for (auto m: data->entity_manager->get_component_list<ModelRef>()) {
				m->model = data->session->resource_manager->load_model(m->filename);

				// automagic components for now...
				if (m->owner->get_component<EdwardTag>()->request_auto_components)
					data->_entity_apply_components(m->owner, m->model->_template->components);
			}
			for (auto t: data->entity_manager->get_component_list<TerrainRef>()) {
				if (t->filename) {
					t->terrain = new Terrain(session->ctx, t->filename);
				}

				// automagic components for now...
				if (t->owner->get_component<EdwardTag>()->request_auto_components)
					data->_entity_apply_components(t->owner, LevelData::auto_terrain_components());
			}


			/*for (auto& e: data->entities) {
				if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
//					session->progress->set(_("Terrains"), (float)i / (float)data->terrains.num / 2.0f);
					e.terrain.load(session, engine.map_dir | e.terrain.filename.with(".map"), true);
				} else if (e.basic_type == MultiViewType::WORLD_OBJECT) {
					//session->progress->set(format(_("Object %d / %d"), i, data->Objects.num), (float)i / (float)data->Objects.num / 2.0f + 0.5f);
					e.object.object = data->session->resource_manager->load_model(e.object.filename);
					if (auto sk = e.object.object->_template->skeleton)
						for (int i=0; i<sk->bones.num; i++)
							if (sk->filename[i]){}
				}
			}*/
			bool system_classes_missing = false;
			for (auto& s: data->meta_data.systems)
				if (s.class_name == "")
					system_classes_missing = true;
			if (system_classes_missing) {
				const auto system_classes = session->plugin_manager->enumerate_classes("ui.Controller");
				for (auto& s: data->meta_data.systems)
					if (s.class_name == "") {
						for (const auto c: system_classes)
							if (c->owner->module->filename == s.filename) {
								s.class_name = c->name;
								//s.variables = c.variables;
							}
					}
			}
		} catch (Exception &e) {
			msg_error("ABORT: " + e.message());
		}
	}




	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
}

void read_components(WorldEntity& o, const xml::Element& e) {
	for (auto &ee: e.elements)
		if (ee.tag == "component") {
			ScriptInstanceData sd;
			sd.filename = ee.value("script", "");
			sd.class_name = ee.value("class", "");
			if (ee.value("var") != "")
				sd.variables = str2vars(ee.value("var", ""));
			for (const auto& a: ee.attributes)
				if (a.key != "script" and a.key != "class" and a.key != "var") {
					msg_write(a.key + " = " + a.value);
					sd.variables.add({a.key, "", a.value});
				}
			o.components.add(sd);
		}
}

void FormatWorld::_load_xml(const Path &filename, DataWorld *data, bool deep) {
	//data->entities.clear();
	data->entity_manager->reset();
	data->meta_data.skybox_files.clear();


	LevelData ld;
	ld.load(filename);

	data->meta_data.background_color = ld.background_color;
	data->meta_data.skybox_files = ld.skybox_filename;


	for (auto& e: ld.objects) {
		auto o = data->_create_entity(e.pos, quaternion::rotation(e.ang));
		o->get_component<EdwardTag>()->request_auto_components = true;
		auto m = data->entity_manager->add_component<ModelRef>(o);
		m->filename = e.filename.with(".model");
		data->_entity_apply_components(o, e.components);
	}
	for (auto& e: ld.cameras) {
		auto o = data->_create_entity(e.pos, quaternion::rotation(e.ang));
		auto c = data->entity_manager->add_component<Camera>(o);
		c->min_depth = e.min_depth;
		c->max_depth = e.max_depth;
		c->exposure = e.exposure;
		c->fov = e.fov;
		c->bloom_factor = e.bloom_factor;
		data->_entity_apply_components(o, e.components);
	}
	for (auto& e: ld.lights) {
		auto o = data->_create_entity(e.pos, quaternion::rotation(e.ang));
		auto l = data->entity_manager->add_component<Light>(o);
		l->light.type = e.type;
		l->light.col = e._color;
		if (l->light.type != yrenderer::LightType::DIRECTIONAL)
			l->light.power = yrenderer::Light::_radius_to_power(e.radius);
		l->light.theta = e.theta;
		l->light.harshness = e.harshness;
		l->light.enabled = e.enabled;
		l->light.allow_shadow = (l->light.type == yrenderer::LightType::DIRECTIONAL);
		data->_entity_apply_components(o, e.components);
	}
	for (auto& e: ld.terrains) {
		auto o = data->_create_entity(e.pos, quaternion::ID);
		o->get_component<EdwardTag>()->request_auto_components = true;
		auto t = data->entity_manager->add_component<TerrainRef>(o);
		t->filename = e.filename.with(".map");
		data->_entity_apply_components(o, e.components);
	}
	for (auto& e: ld.entities) {
		auto o = data->_create_entity(e.pos, e.ang);
		data->_entity_apply_components(o, e.components);
	}
	for (const auto& ll: ld.links) {
		WorldLink l;
		l.type = ll.type;
		l.object[0] = ll.object[0];
		l.object[1] = ll.object[1];
		l.pos = ll.pos;
		l.ang = ll.ang;
		//l.radius = e.value("radius")._float();
		data->links.add(l);
	}

	data->meta_data.physics_enabled = ld.physics_enabled;
	data->meta_data.physics_mode = ld.physics_mode;
	data->meta_data.gravity = ld.gravity;
	data->meta_data.fog.enabled = ld.fog.enabled;
	data->meta_data.fog.mode = (ygfx::FogMode)ld.fog.mode;
	data->meta_data.fog.start = ld.fog.start;
	data->meta_data.fog.end = ld.fog.end;
	data->meta_data.fog.density = 1/ld.fog.distance;
	data->meta_data.fog.col = ld.fog._color;
	for (const auto& ss: ld.systems) {
		ScriptInstanceData s;
		s.filename = ss.filename;
		s.class_name = ss.class_name;
		for (auto &ee: ss.variables) {
			ScriptInstanceDataVariable v;
			v.name = ee.name;
			v.value = ee.value;
			s.variables.add(v);
		}
		data->meta_data.systems.add(s);
	}
}

string phys_mode_name(PhysicsMode m);

#if 0
xml::Element encode_light(WorldEntity &l) {
	auto e = xml::Element("light")
	.witha("type", light_type_canonical(l.light.type))
	.witha("color", c2s(l.light.col))
	.witha("harshness", f2s(l.light.harshness, 4));
	if (l.light.type == yrenderer::LightType::DIRECTIONAL) {
		e.add_attribute("ang", v2s(l.ang.get_angles()));
	} else if (l.light.type == yrenderer::LightType::POINT) {
		e.add_attribute("pos", v2s(l.pos));
		e.add_attribute("radius", f2s(l.light.radius, 3));
	} else if (l.light.type == yrenderer::LightType::CONE) {
		e.add_attribute("pos", v2s(l.pos));
		e.add_attribute("ang", v2s(l.ang.get_angles()));
		e.add_attribute("radius", f2s(l.light.radius, 3));
		e.add_attribute("theta", f2s(l.light.theta, 3));
	}
	for (auto &c: l.components)
		e.add(xml::Element("component")
			.witha("script", c.filename.str())
			.witha("class", c.class_name));
	return e;
}
#endif

void FormatWorld::_save(const Path &filename, DataWorld *data) {
	//return;

	xml::Parser p;
	p.elements.add(xml::Element("world"));
	auto &w = p.elements[0];

	{
	auto meta = xml::Element("meta");
	meta.add(xml::Element("version", "1"));
	auto bg = xml::Element("background")
		.witha("color", c2s(data->meta_data.background_color));
	for (auto &sb: data->meta_data.skybox_files)
		if (!sb.is_empty())
			bg.add(xml::Element("skybox").witha("file", sb.str()));
	meta.add(bg);

	auto phys = xml::Element("physics")
	.witha("enabled", b2s(data->meta_data.physics_enabled))
	.witha("gravity", v2s(data->meta_data.gravity))
	.witha("mode", phys_mode_name(data->meta_data.physics_mode));
	meta.add(phys);

	auto f = xml::Element("fog")
	.witha("enabled", b2s(data->meta_data.fog.enabled))
	.witha("mode", i2s((int)data->meta_data.fog.mode))
	.witha("start", f2s(data->meta_data.fog.start, 3))
	.witha("end", f2s(data->meta_data.fog.end, 3))
	.witha("density", f2s(data->meta_data.fog.density, 6))
	.witha("color", c2s(data->meta_data.fog.col));
	meta.add(f);

	for (auto &s: data->meta_data.systems) {
		auto e = xml::Element("system")
		.witha("file", s.filename.str())
		.witha("class", s.class_name);
		for (auto &v: s.variables)
			e.elements.add(xml::Element("var").witha("name", v.name).witha("value", v.value));
		meta.add(e);
	}
	w.add(meta);
	}


	auto save_component = [] (const ScriptInstanceData& c) {
		auto ee = xml::Element("component");
		if (!c.filename.is_empty() and !c.filename.is_in("y"))
			ee.add_attribute("script", str(c.filename));
		ee.add_attribute("class", c.class_name);
		for (auto &v: c.variables)
			ee.add_attribute(v.name, v.value);
		return ee;
	};

	auto add_components = [save_component] (xml::Element& e, const Array<ScriptInstanceData>& components) {
		for (auto &c: components) {
			e.add(save_component(c));
		}
	};

	auto cont = xml::Element("3d");

	for (const auto& [i,e]: enumerate(data->entity_manager->entities)) {
		xml::Element el;

		el = xml::Element("entity").witha("pos", v2s(e->pos));
		if (e->ang != quaternion::ID)
			el.add_attribute("ang", v2s(e->ang.get_angles()));
		if (i == data->EgoIndex)
			el.add_attribute("role", "ego");
		for (auto c: e->components) {
			if (c->component_type != EdwardTag::_class)
				el.add(save_component(session->plugin_manager->describe_class(c->component_type, c)));
		}

#if 0
		if (e.basic_type == MultiViewType::WORLD_TERRAIN) {
			el = xml::Element("terrain")
			.witha("file", str(e.terrain.filename))
			.witha("pos", v2s(e.pos));
		} else if (e.basic_type == MultiViewType::WORLD_OBJECT) {
			el = xml::Element("object")
			.witha("file", str(e.object.filename))
			.witha("name", e.object.name)
			.witha("pos", v2s(e.pos))
			.witha("ang", v2s(e.ang.get_angles()));
			if (i == data->EgoIndex)
				el.add_attribute("role", "ego");
		} else if (e.basic_type == MultiViewType::WORLD_LIGHT) {
			el = encode_light(e);
		/*} else if (e.basic_type == MultiViewType::WORLD_CAMERA) {
			el = xml::Element("camera")
			.witha("pos", v2s(e.pos))
			.witha("ang", v2s(e.ang.get_angles()))
			.witha("fov", f2s(e.camera.fov, 3))
			.witha("minDepth", f2s(e.camera.min_depth, 3))
			.witha("maxDepth", f2s(e.camera.max_depth, 3))
			.witha("exposure", f2s(e.camera.exposure, 3))
			.witha("bloomFactor", f2s(e.camera.bloom_factor, 3));*/
		} else {
			el = xml::Element("entity")
			.witha("pos", v2s(e.pos))
			.witha("ang", v2s(e.ang.get_angles()));
		}
		add_components(el, e.components);
#endif
		cont.add(el);
	}

	for (auto &l: data->links) {
		auto e = xml::Element("link")
		.witha("a", i2s(l.object[0]))
		.witha("b", i2s(l.object[1]))
		.witha("type", link_type_canonical(l.type))
		.witha("pos", v2s(l.pos))
		//.witha("pivotA", v2s(l.pos - data->objects[l.object[0]].pos))
		//.witha("pivotB", v2s(l.pos - data->objects[l.object[1]].pos))
		.witha("ang", v2s(l.ang));
		cont.add(e);
	}
	w.add(cont);

	p.save(filename);
	return;
}
