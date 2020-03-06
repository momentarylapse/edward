/*
 * FormatWorld.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatWorld.h"
#include "../../Edward.h"
#include "../../x/model.h"
#include "../../x/object.h"
#include "../../x/world.h"
#include "../../meta.h"
#include "../../x/ModelManager.h"
#include "../../lib/xfile/xml.h"

FormatWorld::FormatWorld() : TypedFormat<DataWorld>(FD_WORLD, "world", _("World"), Flag::CANONICAL_READ_WRITE) {
}


static string v2s(const vector &v) {
	return format("%.3f %.3f %.3f", v.x, v.y, v.z);
}

static vector s2v(const string &s) {
	auto x = s.explode(" ");
	return vector(x[0]._float(), x[1]._float(), x[2]._float());
}

// RGBA
static string c2s(const color &c) {
	return format("%.3f %.3f %.3f %.3f", c.r, c.g, c.b, c.a);
}

static color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

void FormatWorld::_load(const string &filename, DataWorld *data, bool deep) {
	data->reset();

	string x = FileReadText(filename);
	if (x[0] == 't')
		_load_old(filename, data, deep);
	else
		_load_xml(filename, data, deep);



	if (deep){
		for (int i=0;i<data->Terrains.num;i++){
			ed->progress->set(_("Terrains"), (float)i / (float)data->Terrains.num / 2.0f);
			data->Terrains[i].load(data->Terrains[i].pos, engine.map_dir + data->Terrains[i].filename + ".map", true);
		}
		for (int i=0;i<data->Objects.num;i++){
			//ed->progress->set(format(_("Object %d / %d"), i, data->Objects.num), (float)i / (float)data->Objects.num / 2.0f + 0.5f);
			data->Objects[i].object = (Object*)ModelManager::load(data->Objects[i].filename);
			data->Objects[i].object->pos = data->Objects[i].pos;
			data->Objects[i].object->ang = quaternion::rotation(data->Objects[i].ang);
//			if (Objects[i].object)
//				GodRegisterModel(Objects[i].object);
		}
	}




	// debug...
	/*if (JustCreateLightMap){
		for (int i=0;i<Object.num;i++)
			Object[i].is_selected = true;
		CreateLightMap(true);
	}*/
}

void FormatWorld::_load_xml(const string &filename, DataWorld *data, bool deep) {
	data->cameras.clear();
	data->lights.clear();

	xml::Parser p;
	p.load(filename);
	auto *meta = p.elements[0].find("meta");
	if (meta) {
		for (auto &e: meta->elements) {
			if (e.tag == "background") {
				data->meta_data.BackGroundColor = s2c(e.value("color"));
			} else if (e.tag == "skybox") {
				data->meta_data.SkyBoxFile.add(e.value("file"));
			} else if (e.tag == "physics") {
				data->meta_data.PhysicsEnabled = e.value("enabled")._bool();
				data->meta_data.Gravity = s2v(e.value("gravity"));
			} else if (e.tag == "fog") {
				data->meta_data.FogEnabled = e.value("enabled")._bool();
				data->meta_data.FogMode = e.value("mode")._int();
				data->meta_data.FogStart = e.value("start")._float();
				data->meta_data.FogEnd = e.value("end")._float();
				data->meta_data.FogDensity = e.value("density")._float();
				data->meta_data.FogColor = s2c(e.value("color"));
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
				c.pos = s2v(e.value("pos"));
				c.ang = s2v(e.value("ang"));
				c.fov = e.value("fov", f2s(pi/4, 3))._float();
				c.min_depth = e.value("minDepth", "1")._float();
				c.max_depth = e.value("maxDepth", "10000")._float();
				c.exposure = e.value("exposure", "1")._float();
				data->cameras.add(c);
			} else if (e.tag == "light") {
				WorldLight l;
				l.mode = (e.value("type") == "point") ? LightMode::POINT : LightMode::DIRECTIONAL;
				l.radius = e.value("radius")._float();
				l.harshness = e.value("harshness")._float();
				l.col = s2c(e.value("color"));
				l.ang = s2v(e.value("ang"));
				data->lights.add(l);
			} else if (e.tag == "terrain") {
				WorldTerrain t;
				t.filename = e.value("file");
				t.pos = s2v(e.value("pos"));
				data->Terrains.add(t);
			} else if (e.tag == "object") {
				WorldObject o;
				o.object = NULL;
				o.filename = e.value("file");
				o.name = e.value("name");
				o.pos = s2v(e.value("pos"));
				o.ang = s2v(e.value("ang"));
				data->Objects.add(o);
			}
		}
	}
}

void FormatWorld::_load_old(const string &filename, DataWorld *data, bool deep) {

	File *f = NULL;
	int ffv;

	try{

		f = FileOpenText(filename);
		data->file_time = f->GetDateModification().time;

	ffv = f->ReadFileFormatVersion();

	if ((ffv==10) or (ffv==9)){ // new format
		// Terrains
		f->read_comment();
		int n = f->read_int();
		for (int i=0;i<n;i++){
			WorldTerrain t;
			t.filename = f->read_str();
			f->read_vector(&t.pos);
			data->Terrains.add(t);
		}
		// Gravitation
		f->read_comment();
		data->meta_data.Gravity.x = f->read_float();
		data->meta_data.Gravity.y = f->read_float();
		data->meta_data.Gravity.z = f->read_float();
		// EgoIndex
		f->read_comment();
		data->EgoIndex = f->read_int();
		// Background
		f->read_comment();
		if (ffv == 9)
			f->read_bool(); // BackGroundColorEnabled
		read_color_argb(f, data->meta_data.BackGroundColor);
		if (ffv==9){
			data->meta_data.SkyBoxFile[0] = f->read_str();
		}else{
			int ns=f->read_int();
			if (ns > data->meta_data.SkyBoxFile.num)
				data->meta_data.SkyBoxFile.resize(ns);
			for (int i=0;i<ns;i++)
				data->meta_data.SkyBoxFile[i] = f->read_str();
		}
		// Fog
		f->read_comment();
		data->meta_data.FogEnabled = f->read_bool();
		data->meta_data.FogMode = f->read_word();
		data->meta_data.FogStart = f->read_float();
		data->meta_data.FogEnd = f->read_float();
		data->meta_data.FogDensity = f->read_float();
		read_color_argb(f, data->meta_data.FogColor);
		// Music
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			data->meta_data.MusicFile.add(f->read_str());
		// Objects
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			WorldObject o;
			o.filename = f->read_str();
			o.name = f->read_str();
			o.pos.x = f->read_float();
			o.pos.y = f->read_float();
			o.pos.z = f->read_float();
			o.ang.x = f->read_float();
			o.ang.y = f->read_float();
			o.ang.z = f->read_float();
			o.object = NULL;
			o.view_stage = 0;
			o.is_selected = false;
			o.is_special = false;
			data->Objects.add(o);
		}
		// Scripts
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++){
			WorldScript s;
			s.filename = f->read_str();
			int nv = f->read_int();
			for (int j=0; j<nv; j++){
				WorldScriptVariable var;
				var.name = f->read_str();
				var.value = f->read_str();
				s.variables.add(var);
			}
			data->meta_data.scripts.add(s);
		}
		// ScriptVars
		f->read_comment();
		n = f->read_int();
		for (int i=0;i<n;i++)
			f->read_float();
		if (f->read_str() != "#"){
			data->lights[0].enabled = f->read_bool();
			color am, am2, di, sp;
			read_color_3i(f, am);
			read_color_3i(f, di);
			read_color_3i(f, sp);
			data->lights[0].ang.x = f->read_float();
			data->lights[0].ang.y = f->read_float();
			f->read_comment();
			read_color_3i(f, am2);
			data->lights[0].col = (am + am2) * 2 + di;
			data->lights[0].harshness = di.r / data->lights[0].col.r;
			if (f->read_str() != "#"){
				data->meta_data.PhysicsEnabled = f->read_bool();
			}
		}

	}else{
		throw Exception(format(_("File %s has a wrong file format: %d (expected: %d - %d)!"), filename.c_str(), ffv, 8, 10));
	}
	FileClose(f);

	}catch(Exception &e){
		FileClose(f);
		throw e;
	}
}

void FormatWorld::_save(const string &filename, DataWorld *data) {
	/*	if (!SaveTerrains())
			return;*/

	xml::Parser p;
	p.elements.add(xml::Element("world"));
	auto &w = p.elements[0];

	{
	auto meta = xml::Element("meta");
	auto bg = xml::Element("background")
		.witha("color", c2s(data->meta_data.BackGroundColor));
	for (string &sb: data->meta_data.SkyBoxFile)
		if (sb.num > 0)
			bg.add(xml::Element("skybox").witha("file", sb));
	meta.add(bg);

	auto phys = xml::Element("physics")
	.witha("enabled", b2s(data->meta_data.PhysicsEnabled))
	.witha("gravity", v2s(data->meta_data.Gravity));
	meta.add(phys);

	auto f = xml::Element("fog")
	.witha("enabled", b2s(data->meta_data.FogEnabled))
	.witha("mode", i2s(data->meta_data.FogMode))
	.witha("start", f2s(data->meta_data.FogStart, 3))
	.witha("end", f2s(data->meta_data.FogEnd, 3))
	.witha("density", f2s(data->meta_data.FogDensity, 6))
	.witha("color", c2s(data->meta_data.FogColor));
	meta.add(f);

	for (auto &s: data->meta_data.scripts) {
		auto e = xml::Element("script")
		.witha("file", s.filename);
		for (auto &v: s.variables)
			e.elements.add(xml::Element("var").witha("name", v.name).witha("value", v.value));
		meta.add(e);
	}
	w.add(meta);
	}

	auto cont = xml::Element("3d");
	for (auto &c: data->cameras) {
		auto e = xml::Element("camera")
		.witha("pos", v2s(c.pos))
		.witha("ang", v2s(c.ang))
		.witha("fov", f2s(c.fov, 3))
		.witha("minDepth", f2s(c.min_depth, 3))
		.witha("maxDepth", f2s(c.max_depth, 3))
		.witha("exposure", f2s(c.exposure, 3));
		cont.add(e);
	}

	for (auto &l: data->lights) {
		auto e = xml::Element("light")
		.witha("type", l.mode == LightMode::POINT ? "point" : "directional")
		.witha("color", c2s(l.col))
		.witha("harshness", f2s(l.harshness, 4))
		.witha("radius", f2s(l.radius, 3))
		.witha("ang", v2s(l.ang));
		cont.add(e);
	}

	for (auto &t: data->Terrains) {
		auto e = xml::Element("terrain")
		.witha("file", t.filename)
		.witha("pos", v2s(t.pos));
		cont.add(e);
	}

	for (auto &o: data->Objects) {
		auto e = xml::Element("object")
		.witha("file", o.filename)
		.witha("name", o.name)
		.witha("pos", v2s(o.pos))
		.witha("ang", v2s(o.ang));
		cont.add(e);
	}
	w.add(cont);

	p.save(filename);
	return;


#if 0
	File *f = FileCreateText(filename);
	f->float_decimals = 6;

	f->WriteFileFormatVersion(false, 10);
	f->write_comment("// Terrains");
	f->write_int(data->Terrains.num);
	for (auto &t: data->Terrains) {
		f->write_str(t.filename);
		f->write_float(t.pos.x);
		f->write_float(t.pos.y);
		f->write_float(t.pos.z);
	}
	f->write_comment("// Gravitation");
	f->write_float(data->meta_data.Gravity.x);
	f->write_float(data->meta_data.Gravity.y);
	f->write_float(data->meta_data.Gravity.z);
	f->write_comment("// EgoIndex");
	f->write_int(data->EgoIndex);
	f->write_comment("// Background");
	write_color_argb(f, data->meta_data.BackGroundColor);
	int ns = 0;
	foreachi(string &sb, data->meta_data.SkyBoxFile, i)
		if (sb.num > 0)
			ns = i + 1;
	f->write_int(ns);
	for (int i=0;i<ns;i++)
		f->write_str(data->meta_data.SkyBoxFile[i]);
	f->write_comment("// Fog");
	f->write_bool(data->meta_data.FogEnabled);
	f->write_word(data->meta_data.FogMode);
	f->write_float(data->meta_data.FogStart);
	f->write_float(data->meta_data.FogEnd);
	f->write_float(data->meta_data.FogDensity);
	write_color_argb(f, data->meta_data.FogColor);
	f->write_comment("// Music");
	f->write_int(data->meta_data.MusicFile.num);
	for (string &m: data->meta_data.MusicFile)
		f->write_str(m);
	f->write_comment("// Objects");
	f->write_int(data->Objects.num);
	for (auto &o: data->Objects) {
		f->write_str(o.filename);
		f->write_str(o.name);
		f->write_float(o.pos.x);
		f->write_float(o.pos.y);
		f->write_float(o.pos.z);
		f->write_float(o.ang.x);
		f->write_float(o.ang.y);
		f->write_float(o.ang.z);
	}
	f->write_comment("// Scripts");
	f->write_int(data->meta_data.scripts.num);
	for (auto &s: data->meta_data.scripts) {
		f->write_str(s.filename);
		f->write_int(s.variables.num);
		for (auto &v: s.variables) {
			f->write_str(v.name);
			f->write_str(v.value);
		}
	}
	f->write_comment("// ScriptVars");
	f->write_int(0);
	f->write_comment("// Sun");
	f->write_bool(data->lights[0].enabled);
	write_color_3i(f, data->lights[0].ambient());
	write_color_3i(f, data->lights[0].diffuse());
	write_color_3i(f, Black);
	f->write_float(data->lights[0].ang.x);
	f->write_float(data->lights[0].ang.y);
	f->write_comment("// Ambient");
	write_color_3i(f, Black);
	f->write_comment("// Physics");
	f->write_bool(data->meta_data.PhysicsEnabled);
	f->write_int(0);
	f->write_comment("#");

	delete(f);
#endif

}
