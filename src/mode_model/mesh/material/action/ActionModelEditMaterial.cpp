/*
 * ActionModelEditMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelEditMaterial.h"
#include "../../../data/ModelMesh.h"
#include <lib/mesh/Polygon.h>
#include <lib/image/image.h>
#include <lib/ygraphics/graphics-impl.h>

#include <assert.h>

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial::Color &_c) {
	index = _index;
	col = _c;
}

void *ActionModelEditMaterial::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	std::swap(col, m->materials[index]->col);
	m->out_material_changed.notify();

	return nullptr;
}



void ActionModelEditMaterial::undo(Data *d) {
	execute(d);
}





ActionModelMaterialAddTexture::ActionModelMaterialAddTexture(int _index) {
	index = _index;
}

void *ActionModelMaterialAddTexture::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	auto tl = new ModelMaterial::TextureLevel;
	tl->reload_image(d->session);
	m->materials[index]->texture_levels.add(tl);


	// correct skin vertices
	// (copy highest texture level when adding more levels)
	int ntl = m->materials[index]->texture_levels.num;
	for (Polygon &p: m->editing_mesh->polygons) {
		if (p.material == index) {
			for (auto &side: p.side)
				side.skin_vertex[ntl] = side.skin_vertex[ntl - 1];
		}
	}


	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialAddTexture::undo(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	m->materials[index]->texture_levels.pop();

	m->out_texture_changed.notify();
}





ActionModelMaterialDeleteTexture::ActionModelMaterialDeleteTexture(int _index, int _level) {
	index = _index;
	level = _level;
}

void *ActionModelMaterialDeleteTexture::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));
	assert((level >= 0) and (level < m->materials[index]->texture_levels.num));

	tl = m->materials[index]->texture_levels.extract(level);


	// TODO: correct skin vertices



	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialDeleteTexture::undo(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	m->materials[index]->texture_levels.insert(tl, level);

	m->out_texture_changed.notify();
}








ActionModelMaterialLoadTexture::ActionModelMaterialLoadTexture(int _index, int _level, const Path &_fn) {
	index = _index;
	level = _level;
	filename = _fn;
}

void *ActionModelMaterialLoadTexture::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	auto &tl = m->materials[index]->texture_levels[level];
	std::swap(tl->filename, filename);
	tl->reload_image(d->session);



	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialLoadTexture::undo(Data *d) {
	execute(d);
}






ActionModelMaterialScaleTexture::ActionModelMaterialScaleTexture(int _index, int _level, int _width, int _height) {
	index = _index;
	level = _level;
	width = _width;
	height = _height;
	image = nullptr;
}

ActionModelMaterialScaleTexture::~ActionModelMaterialScaleTexture() {
	if (image)
		delete image;
}

void *ActionModelMaterialScaleTexture::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	auto tl = m->materials[index]->texture_levels[level];

	if (!image) {
		image = tl->image->scale(width, height);
	}
	auto t = tl->image.give();
	tl->image = image;
	image = t;
	tl->edited = true;
	tl->update_texture();



	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialScaleTexture::undo(Data *d) {
	execute(d);
}
