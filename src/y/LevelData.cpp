/*
 * LevelData.cpp
 *
 *  Created on: 16.08.2020
 *      Author: michi
 */

#include "LevelData.h"
#include "Link.h"
#include "World.h"
#include "../lib/file/file.h"
#include "../lib/xfile/xml.h"
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

color ReadColor3(File *f) {
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
}



static vector s2v(const string &s) {
	auto x = s.explode(" ");
	return vector(x[0]._float(), x[1]._float(), x[2]._float());
}

// RGBA
static color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

bool LevelData::load(const Path &filename) {
	world_filename = filename;

	xml::Parser p;
	p.load(engine.map_dir << filename.with(".world"));
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
				Script s;
				s.filename = e.value("file");
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
				lights.add(l);
			} else if (e.tag == "terrain") {
				Terrain t;
				t.filename = e.value("file");
				t.pos = s2v(e.value("pos"));
				terrains.add(t);
			} else if (e.tag == "object") {
				Object o;
				o.filename = e.value("file");
				o.script = e.value("script");
				o.name = e.value("name");
				o.pos = s2v(e.value("pos"));
				o.ang = s2v(e.value("ang"));
				o.vel = v_0;
				o.rot = v_0;
				if (e.value("role") == "ego")
					ego_index = objects.num;
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
