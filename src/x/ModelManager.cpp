/*
 * ModelManager.cpp
 *
 *  Created on: 19.01.2020
 *      Author: michi
 */

#include "ModelManager.h"
#include "model.h"
#include "../meta.h"

Array<Model*> ModelManager::originals;


Model* ModelManager::load(const string &_filename) {
	if (_filename == "")
		return nullptr;
	string filename = engine.object_dir + _filename + ".model";
	for (auto *o: originals)
		if (o->_template->filename == filename) {
			return o->copy();
		}

	Model *m = new Model();
	m->load(filename);
	originals.add(m);
	return m->copy();
}
