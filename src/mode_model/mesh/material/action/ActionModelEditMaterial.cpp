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
#include <Session.h>
#include <view/MaterialPreviewManager.h>

#include <assert.h>


ActionModelEditMaterial::ActionModelEditMaterial(int _index, const yrenderer::Material &m) {
	index = _index;
	material = m;
}

void *ActionModelEditMaterial::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	std::swap(material, *m->materials[index]);
	m->out_material_changed.notify();
	d->session->material_preview_manager->invalidate(m->materials[index]);

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

	if (!texture)
		texture = new ygfx::Texture(512, 512, "rgba:i8");

	m->materials[index]->textures.add(texture);


	// correct skin vertices
	// (copy highest texture level when adding more levels)
	int ntl = m->materials[index]->textures.num;
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

	m->materials[index]->textures.pop();

	m->out_texture_changed.notify();
}





ActionModelMaterialDeleteTexture::ActionModelMaterialDeleteTexture(int _index, int _level) {
	index = _index;
	level = _level;
}

void *ActionModelMaterialDeleteTexture::execute(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));
	assert((level >= 0) and (level < m->materials[index]->textures.num));

	texture = m->materials[index]->textures[level];
	m->materials[index]->textures.erase(level);


	// TODO: correct skin vertices



	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialDeleteTexture::undo(Data *d) {
	auto *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->materials.num));

	m->materials[index]->textures.insert(texture, level);

	m->out_texture_changed.notify();
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

#if 0
	auto tl = m->materials[index]->texture_levels[level];

	if (!image) {
		image = tl->image->scale(width, height);
	}
	auto t = tl->image.give();
	tl->image = image;
	image = t;
	tl->edited = true;
	tl->update_texture();
#endif


	m->out_texture_changed.notify();
	return nullptr;
}

void ActionModelMaterialScaleTexture::undo(Data *d) {
	execute(d);
}
