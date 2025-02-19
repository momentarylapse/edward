/*
 * FormatModelPly.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModelPly.h"
#include "../../view/EdwardWindow.h"
#include "../../mode_model/data/DataModel.h"
#include "../../mode_model/data/ModelMesh.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"
#include "../../lib/os/msg.h"

FormatModelPly::FormatModelPly(Session *s) : TypedFormat<DataModel>(s, FD_MODEL, "ply", "Model ply", Flag::READ) {
}

void FormatModelPly::_load(const Path &filename, DataModel *m, bool deep) {
	os::fs::FileStream *f = nullptr;

	//m->material.add(new ModelMaterial());

	try {

	f = os::fs::open(filename, "rt");

	struct Element {
		string name;
		int num;
		int properties;
	};
	Array<Element> elements;
	bool binary = false;

	PolygonMesh g;

	// header
	while (true) {
		string s = f->read_str();
		if (s == "ply") {
			continue; // ignore
		} else if (s.head(7) == "comment") {
			continue; // ignore
		} else if (s == "format ascii 1.0") {
			continue; // ignore
		} else if (s == "format binary_little_endian 1.0") {
			binary = true;
		} else if (s.head(8) == "obj_info") {
			continue; // ignore
		} else if (s.head(7) == "element") {
			auto el = s.explode(" ");
			if (el.num != 3)
				throw FormatError("element can't be parsed: " + s);
			Element e;
			e.name = el[1];
			e.num = el[2]._int();
			e.properties = 0;
			elements.add(e);
		} else if (s.head(8) == "property") {
			auto el = s.explode(" ");

		} else if (s == "end_header") {
			break;
		} else {
			throw FormatError("don't understand header line: " + s);
		}
	}

	if (binary) {
		int offset = f->pos();
		f = os::fs::open(filename, "rb");
		f->set_pos(offset);

		for (auto &e: elements) {
			if (e.name == "vertex") {
				g.vertices.resize(e.num);
				for (int i=0; i<e.num; i++)
					f->read_vector(&g.vertices[i].pos);
			} else if (e.name == "face") {
				for (int i=0; i<e.num; i++) {
					int n = f->read_byte();
					int a = f->read_int();
					int b = f->read_int();
					int c = f->read_int();
					g.add_polygon({a, b, c}, {{0,0,0}, {0,0,0}, {0,0,0}});
				}
			} else {
				msg_error(e.name);
			}
		}

	} else {

		for (auto &e: elements) {
			if (e.name == "vertex") {
				g.vertices.resize(e.num);
				for (int i=0; i<e.num; i++) {
					string t = f->read_str();
					auto tt = t.explode(" ");
					if (tt.num < 3)
						continue;
					g.vertices[i].pos.x = tt[0]._float();
					g.vertices[i].pos.y = tt[1]._float();
					g.vertices[i].pos.z = tt[2]._float();
				}
			} else if (e.name == "face") {


				for (int i=0; i<e.num; i++) {
					string t = f->read_str();
					auto tt = t.explode(" ");
					if (tt.num < 3)
						continue;
					int n = tt[0]._int();
					if (n < 3)
						continue;
					/*ModelPolygon p;
					p.is_selected = false;
					p.triangulation_dirty = true;
					p.material = 0;
					p.side.resize(n);
					for (int k=0;k<n;k++) {
						p.side[k].vertex = tt[1+k]._int();
					}
					p.normal_dirty = true;
					s.polygon.add(p);*/
					g.add_polygon({tt[1]._int(), tt[2]._int(), tt[3]._int()}, {{0,0,0}, {0,0,0}, {0,0,0}});
				}
			} else {
				// ignore
				for (int i=0; i<e.num; i++)
					f->read_str();
			}
		}
	}
#if 0
		m->pasteGeometry(g, 0);
#else
		throw Exception("todo");
#endif

	} catch (Exception &e) {
		delete f;
		throw e;
	}

	if (f)
		delete f;
}

void FormatModelPly::_save(const Path &filename, DataModel *data) {
}
