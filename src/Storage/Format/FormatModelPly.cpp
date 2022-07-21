/*
 * FormatModelPly.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModelPly.h"
#include "../../Edward.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/ModelMesh.h"
#include "../../Data/Model/ModelPolygon.h"
#include "../../lib/os/file.h"
#include "../../lib/os/formatter.h"

FormatModelPly::FormatModelPly() : TypedFormat<DataModel>(FD_MODEL, "ply", _("Model ply"), Flag::READ) {
}

void FormatModelPly::_load(const Path &filename, DataModel *m, bool deep) {
	TextLinesFormatter *f = nullptr;

	//m->material.add(new ModelMaterial());

	try {

	f = new TextLinesFormatter(os::fs::open(filename, "rt"));

	struct Element {
		string name;
		int num;
		int properties;
	};
	Array<Element> elements;

	// header
	while (true) {
		string s = f->read_str();
		if (s == "ply") {
			continue; // ignore
		} else if (s.head(7) == "comment") {
			continue; // ignore
		} else if (s == "format ascii 1.0") {
			continue; // ignore
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

	for (auto &e: elements) {
		if (e.name == "vertex") {
			m->mesh->vertex.resize(e.num);
			for (int i=0; i<e.num; i++) {
				string t = f->read_str();
				auto tt = t.explode(" ");
				if (tt.num < 3)
					continue;
				m->mesh->vertex[i].pos.x = tt[0]._float();
				m->mesh->vertex[i].pos.y = tt[1]._float();
				m->mesh->vertex[i].pos.z = tt[2]._float();
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
				try{
					m->addPolygon({tt[1]._int(), tt[2]._int(), tt[3]._int()}, 0);
				}catch(...){
				}
			}
		} else {
			// ignore
			for (int i=0; i<e.num; i++)
				f->read_str();
		}
	}

	} catch (Exception &e) {
		delete f;
		throw e;
	}

	if (f)
		delete(f);
}

void FormatModelPly::_save(const Path &filename, DataModel *data) {
}
