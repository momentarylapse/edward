/*
 * LevelData.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "LevelData.h"
#include "components/Link.h"
#include "World.h"
#include "systems/Physics.h"
#include "../plugins/PluginManager.h"
#include <lib/doc/xml.h>
#include <lib/yrenderer/scene/Light.h>
#include <lib/os/file.h>


LevelData::LevelData() {
	background_color = Gray;

	gravity = v_0;
	fog.enabled = false;
	physics_enabled = false;
	physics_mode = PhysicsMode::FULL_EXTERNAL;
}


bool LevelData::load(const Path& filename) {
	world_filename = filename;

	using namespace PluginManager;

	xml::Parser p;
	p.load(world_filename);
	if (auto meta = p.elements[0].find("meta")) {
		for (auto &e: meta->elements) {
			if (e.tag == "background") {
				background_color = s2c(e.value("color"));
				for (auto &ee: e.elements) {
					if (ee.tag == "skybox") {
						skybox_filename.add(ee.value("file"));
						skybox_ang.add(v_0);
					}
				}
			} else if (e.tag == "physics") {
				physics_enabled = e.value("enabled")._bool();
				if (e.value("mode") == "simple")
					physics_mode = PhysicsMode::SIMPLE;
				else if (e.value("mode") == "full")
					physics_mode = PhysicsMode::FULL_EXTERNAL;
				gravity = s2v(e.value("gravity"));
			} else if (e.tag == "fog") {
				fog.enabled = e.value("enabled")._bool();
				fog.mode = e.value("mode")._int();
				fog.start = e.value("start")._float();
				fog.end = e.value("end")._float();
				fog.distance = 1.0f / e.value("density")._float();
				fog._color = s2c(e.value("color"));
			} else if (e.tag == "script" or e.tag == "system") {
				ScriptInstanceData s;
				s.filename = e.value("file");
				s.class_name = e.value("class");

				// deprecated
				const string var = e.value("var");
				if (var.num > 0)
					s.variables = parse_variables(var);

				for (auto &ee: e.elements) {
					ScriptInstanceDataVariable v;
					v.name = ee.value("name");//.lower().replace("_", "");
					v.value = ee.value("value");
					s.variables.add(v);
				}
				systems.add(s);
			}
		}
	}

	auto read_components = [] (Array<ScriptInstanceData>& components, xml::Element &e) {
		for (const auto &ee: e.elements)
			if (ee.tag == "component") {
				ScriptInstanceData sd;
				sd.filename = ee.value("script");
				sd.class_name = ee.value("class");

				// deprecated...
				const string var = ee.value("var");
				if (var.num > 0)
					sd.variables = PluginManager::parse_variables(var);

				for (const auto& a: ee.attributes)
					if (a.key != "script" and a.key != "class" and a.key != "var")
						sd.variables.add({a.key, Any::parse(a.value)});
				components.add(sd);
			}
	};


	if (auto cont = p.elements[0].find("3d")) {
		for (auto &e: cont->elements) {
			if (e.tag == "camera") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				ScriptInstanceData c = {"Camera"};
				c.set("fov", e.value("fov", f2s(pi/4, 3))._float());
				c.set("min_depth", e.value("minDepth", "1")._float());
				c.set("max_depth", e.value("maxDepth", "10000")._float());
				c.set("exposure", e.value("exposure", "1")._float());
				c.set("bloom_factor", e.value("bloomFactor", "0.15")._float());
				o.components.add(c);
				read_components(o.components, e);
				entities.add(o);
			} else if (e.tag == "light") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				ScriptInstanceData l = {"Light"};
				l.set("harshness", e.value("harshness")._float());
				l.set("color", e.value("color"));
				l.set("power", e.value("power", "1.0")._float());
				l.set("theta", -1.0);
				l.set("allow_shadow", false);
				if (e.value("type") == "directional") {
					l.set("type", (int)yrenderer::LightType::DIRECTIONAL);
					l.set("allow_shadow", true);
				} else if (e.value("type") == "point") {
					l.set("type", (int)yrenderer::LightType::POINT);
					if (e.has_value("radius"))
						l.set("power", yrenderer::Light::_radius_to_power(e.value("radius")._float()));
				} else if (e.value("type") == "cone") {
					l.set("type", (int)yrenderer::LightType::CONE);
					if (e.has_value("radius"))
						l.set("power", yrenderer::Light::_radius_to_power(e.value("radius")._float()));
					l.set("theta", e.value("theta")._float());
				}
				l.set("enabled", e.value("enabled", "true")._bool());
				o.components.add(l);
				read_components(o.components, e);
				entities.add(o);
			} else if (e.tag == "terrain") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				ScriptInstanceData t = {"TerrainRef"};
				t.set("terrain", e.value("file"));
				t.set("material", e.value("material"));
				o.components.add(t);
				o.components.append(auto_terrain_components());
				read_components(o.components, e);
				entities.add(o);
			} else if (e.tag == "object") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				ScriptInstanceData t = {"TemplateRef"};
				t.set("template", e.value("file"));
				o.components.add(t);
				if (e.value("role") == "ego")
					o.components.add({"EgoMarker", "", {}});
				read_components(o.components, e);
				entities.add(o);
			} else if (e.tag == "entity") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				if (e.value("role") == "ego")
					o.components.add({"EgoMarker", "", {}});
				read_components(o.components, e);
				entities.add(o);
			} else if (e.tag == "link") {
				Entity o;
				o.pos = s2v(e.value("pos"));
				o.ang = quaternion::rotation(s2v(e.value("ang")));
				ScriptInstanceData l = {"Link"};
				auto type = LinkType::SOCKET;
				if (e.value("type") == "hinge")
					type = LinkType::HINGE;
				else if (e.value("type") == "universal")
					type = LinkType::UNIVERSAL;
				else if (e.value("type") == "spring")
					type = LinkType::SPRING;
				l.set("type", (int)type);
				l.set("a", e.value("a", "-1")._int());
				l.set("b", e.value("b", "-1")._int());
				o.components.add(l);
				read_components(o.components, e);
				entities.add(o);
			}
		}
	}

	return true;
}


#if 0
static string v2s(const vec3 &v) {
	return format("%.3f %.3f %.3f", v.x, v.y, v.z);
}

// RGBA
static string c2s(const color &c) {
	return format("%.3f %.3f %.3f %.3f", c.r, c.g, c.b, c.a);
}
#endif

string phys_mode_name(PhysicsMode m) {
	if (m == PhysicsMode::SIMPLE)
		return "simple";
	if (m == PhysicsMode::FULL_EXTERNAL)
		return "full";
	return "";
}

#include "Model.h"
#include "Terrain.h"
#include "components/Camera.h"
#include <ecs/Entity.h>
#include <ecs/ComponentManager.h>
#ifdef _X_ALLOW_X_
#include "../plugins/PluginManager.h"
#include "../plugins/Controller.h"
#endif
#include <lib/kaba/kaba.h>

void LevelData::save(const Path &filename) {
#ifdef _X_ALLOW_X_
	xml::Parser p;

	p.elements.add(xml::Element("world"));
	auto &w = p.elements[0];

	{
	auto meta = xml::Element("meta");
	auto bg = xml::Element("background")
		.witha("color", c2s(world.background));
	for (auto sb: world.skybox)
		if (sb)
			bg.add(xml::Element("skybox").witha("file", sb->filename().str()));
	meta.add(bg);

	auto phys = xml::Element("physics")
	.witha("enabled", b2s(world.physics_mode != PhysicsMode::NONE))
	.witha("gravity", v2s(world.gravity))
	.witha("mode", phys_mode_name(world.physics_mode));
	meta.add(phys);

	auto f = xml::Element("fog")
	.witha("enabled", b2s(world.fog.enabled))
	.witha("mode", i2s((int)world.fog.mode))
	.witha("start", f2s(world.fog.start, 3))
	.witha("end", f2s(world.fog.end, 3))
	.witha("distance", f2s(world.fog.distance, 6))
	.witha("color", c2s(world.fog._color));
	meta.add(f);

	for (auto s: PluginManager::controllers) {
		//s->_class->owner->script->filename
		auto e = xml::Element("script")
		.witha("file", s->_class->owner->script->filename.relative_to(kaba::config.directory).str());
		//for (auto &v: s.variables)
		//	e.elements.add(xml::Element("var").witha("name", v.name).witha("value", v.value));
		meta.add(e);
	}
	w.add(meta);
	}

	auto cont = xml::Element("3d");
	auto cameras = ComponentManager::get_listx<::Camera>();
	for (auto c: *cameras) {
		auto o = c->get_owner<Entity3D>();
		auto e = xml::Element("camera")
		.witha("pos", v2s(o->pos))
		.witha("ang", v2s(o->ang.get_angles()))
		.witha("fov", f2s(c->fov, 3))
		.witha("minDepth", f2s(c->min_depth, 3))
		.witha("maxDepth", f2s(c->max_depth, 3))
		.witha("exposure", f2s(c->exposure, 3));
		/*for (auto &com: o->components)
			e.add(xml::Element("component")
				.witha("script", com.filename.str())
				.witha("class", com.class_name));*/
		cont.add(e);
	}

	//for (auto l: world.lights)
	//	cont.add(encode_light(l));

	for (auto t: world.terrains) {
		auto o = t->get_owner<Entity3D>();
		auto e = xml::Element("terrain")
		.witha("file", t->filename.str())
		.witha("pos", v2s(o->pos));
		/*for (auto &c: t.components)
			e.add(xml::Element("component")
				.witha("script", c.filename.str())
				.witha("class", c.class_name));*/
		cont.add(e);
	}

	/*for (auto o: world.objects) {
		auto e = xml::Element("object")
		.witha("file", o.filename.str())
		.witha("name", o.name)
		.witha("pos", v2s(o.pos))
		.witha("ang", v2s(o.ang));
		for (auto &c: o.components)
			e.add(xml::Element("component")
				.witha("script", c.filename.str())
				.witha("class", c.class_name));
		//if (!o.script.is_empty() and o.script != o.object->_template->script_filename)
		//	e.add_attribute("script", o.script.str());
		if (i == data->EgoIndex)
			e.add_attribute("role", "ego");
		cont.add(e);
	}*/

	/*for (auto &l: world.links) {
		auto e = xml::Element("link")
		.witha("a", i2s(l.object[0]))
		.witha("b", i2s(l.object[1]))
		.witha("type", link_type_canonical(l.type))
		.witha("pos", v2s(l.pos))
		//.witha("pivotA", v2s(l.pos - data->objects[l.object[0]].pos))
		//.witha("pivotB", v2s(l.pos - data->objects[l.object[1]].pos))
		.witha("ang", v2s(l.ang));
		for (auto &c: l.components)
			e.add(xml::Element("component")
				.witha("script", c.filename.str())
				.witha("class", c.class_name));
		cont.add(e);
	}*/
	w.add(cont);

	p.save(filename);
#endif
}

Array<ScriptInstanceData> LevelData::auto_terrain_components() {
	return {{"TerrainCollider", "", {{}}},
		{"RigidBody", "", {{"dynamic", false}}}};
}




