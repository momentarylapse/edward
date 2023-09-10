/*
 * FormatWorld.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatWorld.h"
#include "../../data/world/DataWorld.h"
#include "../../data/world/WorldCamera.h"
#include "../../data/world/WorldLight.h"
#include "../../data/world/WorldLink.h"
#include "../../data/world/WorldObject.h"
#include "../../data/world/WorldTerrain.h"
#include "../../EdwardWindow.h"
#include "../../stuff/Progress.h"
#include "../../y/Model.h"
#include "../../y/Object.h"
#include "../../y/World.h"
#include "../../y/EngineData.h"
#include "../../y/ModelManager.h"
#include "../../y/components/Skeleton.h"
#include "../../lib/os/date.h"
#include "../../lib/os/file.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/formatter.h"
#include "../../lib/doc/xml.h"

FormatWorld::FormatWorld(EdwardWindow *ed) : TypedFormat<DataWorld>(ed, FD_WORLD, "world", _("World"), Flag::CANONICAL_READ_WRITE) {
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
string vars2str(const Array<WorldScriptVariable>& vars) {
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

Array<WorldScriptVariable> str2vars(const string& s) {
	Array<WorldScriptVariable> vars;
	for (auto &x: s.explode(",")) {
		auto xx = x.explode(":");
		if (xx.num != 2)
			continue;
		WorldScriptVariable v;
		v.name = xx[0].trim();
		v.value = xx[1].trim();
		vars.add(v);
	}
	return vars;
}

void FormatWorld::_load(const Path &filename, DataWorld *data, bool deep) {
	data->reset();

	string x = os::fs::read_text(filename);
	msg_write(x.head(10));
	if (x[0] == 't')
		_load_old(filename, data, deep);
	else
		_load_xml(filename, data, deep);



	if (deep) {
		try {
		for (int i=0;i<data->terrains.num;i++) {
			ed->progress->set(_("Terrains"), (float)i / (float)data->terrains.num / 2.0f);
			data->terrains[i].load(engine.map_dir | data->terrains[i].filename.with(".map"), true);
		}
		for (int i=0;i<data->objects.num;i++) {
			//ed->progress->set(format(_("Object %d / %d"), i, data->Objects.num), (float)i / (float)data->Objects.num / 2.0f + 0.5f);
			data->objects[i].object = (Object*)ModelManager::load(data->objects[i].filename);
			if (auto sk = data->objects[i].object->_template->skeleton) {
				for (int i=0; i<sk->bones.num; i++)
					if (sk->filename[i]){}
			}
//			if (Objects[i].object)
//				GodRegisterModel(Objects[i].object);
		}
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}




	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
}

void FormatWorld::_load_xml(const Path &filename, DataWorld *data, bool deep) {
	data->cameras.clear();
	data->lights.clear();
	data->meta_data.skybox_files.clear();

	xml::Parser p;
	p.load(filename);
	auto *meta = p.elements[0].find("meta");
	if (meta) {
		for (auto &e: meta->elements) {
			if (e.tag == "background") {
				data->meta_data.background_color = s2c(e.value("color", "0 0 0"));
				for (auto &ee: e.elements) {
					if (ee.tag == "skybox")
						data->meta_data.skybox_files.add(ee.value("file"));
				}
			} else if (e.tag == "physics") {
				data->meta_data.physics_enabled = e.value("enabled", "true")._bool();
				if (e.value("mode") == "full")
					data->meta_data.physics_mode = PhysicsMode::FULL_EXTERNAL;
				else if (e.value("mode") == "simple")
					data->meta_data.physics_mode = PhysicsMode::SIMPLE;
				data->meta_data.gravity = s2v(e.value("gravity", "0 0 0"));
			} else if (e.tag == "fog") {
				data->meta_data.fog.enabled = e.value("enabled", "false")._bool();
				data->meta_data.fog.mode = (nix::FogMode)e.value("mode", "0")._int();
				data->meta_data.fog.start = e.value("start", "0")._float();
				data->meta_data.fog.end = e.value("end", "10000")._float();
				data->meta_data.fog.density = e.value("density", "0")._float();
				data->meta_data.fog.col = s2c(e.value("color", "0 0 0"));
			} else if (e.tag == "script") {
				WorldScript s;
				s.filename = e.value("file");
				for (auto &ee: e.elements) {
					WorldScriptVariable v;
					v.name = ee.value("name");
					v.value = ee.value("value");
					s.variables.add(v);
				}
				data->meta_data.scripts.add(s);
			}
		}
	}


	auto *cont = p.elements[0].find("3d");
	if (cont) {
		for (auto &e: cont->elements) {
			if (e.tag == "camera") {
				WorldCamera c;
				c.pos = s2v(e.value("pos", "0 0 0"));
				c.ang = s2v(e.value("ang", "0 0 0"));
				c.fov = e.value("fov", f2s(pi/4, 3))._float();
				c.min_depth = e.value("minDepth", "1")._float();
				c.max_depth = e.value("maxDepth", "10000")._float();
				c.exposure = e.value("exposure", "1")._float();
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptInstanceData sd;
						sd.filename = ee.value("script", "");
						sd.class_name = ee.value("class", "");
						c.components.add(sd);
					}
				data->cameras.add(c);
			} else if (e.tag == "light") {
				WorldLight l;
				l.type = LightType::POINT;
				if (e.value("type") == "directional")
					l.type = LightType::DIRECTIONAL;
				else if (e.value("type") == "cone")
					l.type = LightType::CONE;
				l.radius = e.value("radius", "0")._float();
				l.theta = e.value("theta", "0")._float();
				l.harshness = e.value("harshness", "0.8")._float();
				l.col = s2c(e.value("color", "1 1 1"));
				l.ang = s2v(e.value("ang", "0 0 0"));
				l.pos= s2v(e.value("pos", "0 0 0"));
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptInstanceData sd;
						sd.filename = ee.value("script", "");
						sd.class_name = ee.value("class", "");
						l.components.add(sd);
					}
				data->lights.add(l);
			} else if (e.tag == "terrain") {
				WorldTerrain t;
				t.filename = e.value("file");
				t.pos = s2v(e.value("pos", "0 0 0"));
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptInstanceData sd;
						sd.filename = ee.value("script", "");
						sd.class_name = ee.value("class", "");
						sd.variables = str2vars(ee.value("var", ""));
						t.components.add(sd);
					}
				data->terrains.add(t);
			} else if (e.tag == "object") {
				WorldObject o;
				o.object = NULL;
				o.filename = e.value("file");
				o.name = e.value("name");
				//o.script = e.value("script");
				o.pos = s2v(e.value("pos", "0 0 0"));
				o.ang = s2v(e.value("ang", "0 0 0"));
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptInstanceData sd;
						sd.filename = ee.value("script", "");
						sd.class_name = ee.value("class", "");
						sd.variables = str2vars(ee.value("var", ""));
						o.components.add(sd);
					}
				if (e.value("role") == "ego")
					data->EgoIndex = data->objects.num;
				data->objects.add(o);
			} else if (e.tag == "link") {
				WorldLink l;
				l.type = LinkType::SOCKET;
				if (e.value("type") == "hinge")
					l.type = LinkType::HINGE;
				if (e.value("type") == "spring")
					l.type = LinkType::SPRING;
				if (e.value("type") == "universal")
					l.type = LinkType::UNIVERSAL;
				l.object[0] = e.value("a")._int();
				l.object[1] = e.value("b")._int();
				l.pos = s2v(e.value("pos", "0 0 0"));
				l.ang = s2v(e.value("ang", "0 0 0"));
				for (auto &ee: e.elements)
					if (ee.tag == "component") {
						ScriptInstanceData sd;
						sd.filename = ee.value("script", "");
						sd.class_name = ee.value("class", "");
						l.components.add(sd);
					}
				//l.radius = e.value("radius")._float();
				data->links.add(l);
			} else {
				msg_error("unhandled tag: " + e.tag);
			}
		}
	}
}

string phys_mode_name(PhysicsMode m);

xml::Element encode_light(WorldLight &l) {
	auto e = xml::Element("light")
	.witha("type", light_type_canonical(l.type))
	.witha("color", c2s(l.col))
	.witha("harshness", f2s(l.harshness, 4));
	if (l.type == LightType::DIRECTIONAL) {
		e.add_attribute("ang", v2s(l.ang));
	} else if (l.type == LightType::POINT) {
		e.add_attribute("pos", v2s(l.pos));
		e.add_attribute("radius", f2s(l.radius, 3));
	} else if (l.type == LightType::CONE) {
		e.add_attribute("pos", v2s(l.pos));
		e.add_attribute("ang", v2s(l.ang));
		e.add_attribute("radius", f2s(l.radius, 3));
		e.add_attribute("theta", f2s(l.theta, 3));
	}
	for (auto &c: l.components)
		e.add(xml::Element("component")
			.witha("script", c.filename.str())
			.witha("class", c.class_name));
	return e;
}

void FormatWorld::_save(const Path &filename, DataWorld *data) {

	xml::Parser p;
	p.elements.add(xml::Element("world"));
	auto &w = p.elements[0];

	{
	auto meta = xml::Element("meta");
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

	for (auto &s: data->meta_data.scripts) {
		auto e = xml::Element("script")
		.witha("file", s.filename.str());
		for (auto &v: s.variables)
			e.elements.add(xml::Element("var").witha("name", v.name).witha("value", v.value));
		meta.add(e);
	}
	w.add(meta);
	}

	auto add_components = [] (xml::Element& e, const Array<ScriptInstanceData>& components) {
		for (auto &c: components)
			e.add(xml::Element("component")
				.witha("script", c.filename.str())
				.witha("class", c.class_name)
				.witha("var", vars2str(c.variables)));
	};

	auto cont = xml::Element("3d");
	for (auto &c: data->cameras) {
		auto e = xml::Element("camera")
		.witha("pos", v2s(c.pos))
		.witha("ang", v2s(c.ang))
		.witha("fov", f2s(c.fov, 3))
		.witha("minDepth", f2s(c.min_depth, 3))
		.witha("maxDepth", f2s(c.max_depth, 3))
		.witha("exposure", f2s(c.exposure, 3));
		add_components(e, c.components);
		cont.add(e);
	}

	for (auto &l: data->lights)
		cont.add(encode_light(l));

	for (auto &t: data->terrains) {
		auto e = xml::Element("terrain")
		.witha("file", t.filename.str())
		.witha("pos", v2s(t.pos));
		add_components(e, t.components);
		cont.add(e);
	}

	foreachi (auto &o, data->objects, i) {
		auto e = xml::Element("object")
		.witha("file", o.filename.str())
		.witha("name", o.name)
		.witha("pos", v2s(o.pos))
		.witha("ang", v2s(o.ang));
		add_components(e, o.components);
		if (i == data->EgoIndex)
			e.add_attribute("role", "ego");
		cont.add(e);
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
		add_components(e, l.components);
		cont.add(e);
	}
	w.add(cont);

	p.save(filename);
	return;
}
