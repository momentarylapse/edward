/*
 * ActionModelEditMaterial.cpp
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#include <algorithm>
#include "ActionModelEditMaterial.h"
#include <assert.h>

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial::Color &_c)
{
	index = _index;
	mode = 0;
	col = _c;
}

ActionModelEditMaterial::ActionModelEditMaterial(int _index, const ModelMaterial::Alpha &_a)
{
	index = _index;
	mode = 1;
	alpha = _a;
}

void *ActionModelEditMaterial::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));

	if (mode == 0) {
		std::swap(col, m->material[index]->col);
	} else if (mode == 1) {
		std::swap(alpha, m->material[index]->alpha);
	} else if (mode == 2) {

/*
	// correct skin vertices
	// (copy highest texture level when adding more levels)
	for (ModelSurface &s: m->surface)
		for (ModelPolygon &p: s.polygon){
			if (p.material == index){
				for (ModelPolygonSide &side: p.side)
					for (int l=mat.texture_levels.num;l<m->material[index].texture_levels.num;l++)
						side.skin_vertex[l] = side.skin_vertex[mat.texture_levels.num - 1];
			}
		}*/
	}
	m->notify(m->MESSAGE_MATERIAL_CHANGE);

	return NULL;
}



void ActionModelEditMaterial::undo(Data *d)
{
	execute(d);
}





ActionModelMaterialAddTexture::ActionModelMaterialAddTexture(int _index) {
	index = _index;
}

void *ActionModelMaterialAddTexture::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));

	auto *tl = new ModelMaterial::TextureLevel();
	tl->reload_image();
	m->material[index]->texture_levels.add(tl);


	// correct skin vertices
	// (copy highest texture level when adding more levels)
	int ntl = m->material[index]->texture_levels.num;
	for (ModelSurface &s: m->surface)
		for (ModelPolygon &p: s.polygon) {
			if (p.material == index) {
				for (ModelPolygonSide &side: p.side)
					side.skin_vertex[ntl] = side.skin_vertex[ntl - 1];
			}
		}


	m->notify(m->MESSAGE_TEXTURE_CHANGE);
	return NULL;
}

void ActionModelMaterialAddTexture::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));

	auto tl = m->material[index]->texture_levels.pop();
	delete tl;

	m->notify(m->MESSAGE_TEXTURE_CHANGE);
}





ActionModelMaterialDeleteTexture::ActionModelMaterialDeleteTexture(int _index, int _level) {
	index = _index;
	level = _level;
	tl = nullptr;
}

void *ActionModelMaterialDeleteTexture::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));
	assert((level >= 0) and (level < m->material[index]->texture_levels.num));

	tl = m->material[index]->texture_levels[level];
	m->material[index]->texture_levels.erase(level);


	// TODO: correct skin vertices



	m->notify(m->MESSAGE_TEXTURE_CHANGE);
	return NULL;
}

void ActionModelMaterialDeleteTexture::undo(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));

	m->material[index]->texture_levels.insert(tl, level);

	m->notify(m->MESSAGE_TEXTURE_CHANGE);
}








ActionModelMaterialLoadTexture::ActionModelMaterialLoadTexture(int _index, int _level, const string &_fn) {
	index = _index;
	level = _level;
	filename = _fn;
}

void *ActionModelMaterialLoadTexture::execute(Data *d) {
	DataModel *m = dynamic_cast<DataModel*>(d);
	assert((index >= 0) and (index < m->material.num));

	auto *tl = m->material[index]->texture_levels[level];
	std::swap(tl->filename, filename);
	tl->reload_image();



	m->notify(m->MESSAGE_TEXTURE_CHANGE);
	return NULL;
}

void ActionModelMaterialLoadTexture::undo(Data *d) {
	execute(d);
}
