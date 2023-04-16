/*
 * LevelData.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "LevelData.h"
#include "Link.h"
#include "World.h"
#include "../lib/os/file.h"
#include "../lib/doc/xml.h"
#include "../y/EngineData.h"
#include "../meta.h"


LevelData::LevelData() {
	ego_index = -1;
	background_color = Gray;

	gravity = v_0;
	fog.enabled = false;
	physics_enabled = false;
	physics_mode = PhysicsMode::FULL_EXTERNAL;
}

/*color ReadColor3(Formatter *f) {
	int c[3];
	for (int i=0;i<3;i++)
		c[i] = f->read_float();
	return color::from_int_rgb(c);
}

color ReadColor4(File *f) {
	int c[4];
	for (int i=0;i<4;i++)
		c[i] = f->read_float();
	return color::from_int_argb(c);
}*/



static vec3 s2v(const string &s) {
	auto x = s.explode(" ");
	return vec3(x[0]._float(), x[1]._float(), x[2]._float());
}

// RGBA
static color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

bool LevelData::load(const Path &filename) {
	world_filename = filename;

	xml::Parser p;
	p.load(filename);
	auto *meta = p.elements[0].find("meta");
	if (meta) {
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
			} else if (e.tag == "script") {
				ScriptData s;
				s.filename = e.value("file");
				s.class_name = e.value("class");
				s.var = e.value("var");
				for (auto &ee: e.elements) {
					TemplateDataScriptVariable v;
					v.name = ee.value("name").lower().replace("_", "");
					v.value = ee.value("value");
					s.variables.add(v);
				}
				scripts.add(s);
			}
		}
	}


	auto *cont = p.elements[0].find("3d");
	if (cont) {
		for (auto &e: cont->elements) {
			if (e.tag == "camera") {
				Camera c;
				c.pos = s2v(e.value("pos"));
				c.ang = s2v(e.value("ang"));
				c.fov = e.value("fov", f2s(pi/4, 3))._float();
				c.min_depth = e.value("minDepth", "1")._float();
				c.max_depth = e.value("maxDepth", "10000")._float();
				c.exposure = e.value("exposure", "1")._float();
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptData sd;
						sd.filename = ee.value("script");
						sd.class_name = ee.value("class");
						sd.var = ee.value("var");
						c.components.add(sd);
					}
				cameras.add(c);
			} else if (e.tag == "light") {
				Light l;
				l.harshness = e.value("harshness")._float();
				l._color = s2c(e.value("color"));
				l.radius = -1;
				l.theta = -1;
				if (e.value("type") == "directional") {
					l.ang = s2v(e.value("ang"));
				} else if (e.value("type") == "point") {
					l.pos= s2v(e.value("pos"));
					l.radius = e.value("radius")._float();
					l._color *= l.radius * l.radius / 100;
				} else if (e.value("type") == "cone") {
					l.pos= s2v(e.value("pos"));
					l.ang = s2v(e.value("ang"));
					l.radius = e.value("radius")._float();
					l.theta = e.value("theta")._float();
					l._color *= l.radius * l.radius / 100;
				}
				l.enabled = e.value("enabled", "true")._bool();
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptData sd;
						sd.filename = ee.value("script");
						sd.class_name = ee.value("class");
						sd.var = ee.value("var");
						l.components.add(sd);
					}
				lights.add(l);
			} else if (e.tag == "terrain") {
				Terrain t;
				t.filename = e.value("file");
				t.pos = s2v(e.value("pos"));
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptData sd;
						sd.filename = ee.value("script");
						sd.class_name = ee.value("class");
						sd.var = ee.value("var");
						t.components.add(sd);
					}
				terrains.add(t);
			} else if (e.tag == "object") {
				Object o;
				o.filename = e.value("file");
				o.name = e.value("name");
				o.pos = s2v(e.value("pos"));
				o.ang = s2v(e.value("ang"));
				if (e.value("role") == "ego")
					ego_index = objects.num;
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptData sd;
						sd.filename = ee.value("script");
						sd.class_name = ee.value("class");
						sd.var = ee.value("var");
						o.components.add(sd);
					}
				objects.add(o);
			} else if (e.tag == "link") {
				Link l;
				l.pos = s2v(e.value("pos"));
				l.ang = s2v(e.value("ang"));
				l.object[0] = e.value("a")._int();
				l.object[1] = e.value("b")._int();
				l.type = LinkType::SOCKET;
				if (e.value("type") == "hinge")
					l.type = LinkType::HINGE;
				if (e.value("type") == "universal")
					l.type = LinkType::UNIVERSAL;
				if (e.value("type") == "spring")
					l.type = LinkType::SPRING;
				links.add(l);
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
#include "Camera.h"
#include "../y/Entity.h"
#include "../y/ComponentManager.h"
#ifdef _X_ALLOW_X_
#include "../plugins/PluginManager.h"
#include "../plugins/Controller.h"
#endif
#include "../lib/kaba/kaba.h"

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
