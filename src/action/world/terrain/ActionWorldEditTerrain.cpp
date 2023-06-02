/*
 * ActionWorldEditTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldEditTerrain.h"
#include "../../../data/world/DataWorld.h"
#include "../../../y/Terrain.h"
#include "../../../y/ResourceManager.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

ActionWorldEditTerrain::ActionWorldEditTerrain(int _index, const WorldEditingTerrain &_data) {
	index = _index;
	data = _data;
}

void ActionWorldEditTerrain::undo(Data *d) {
	execute(d);
}



void *ActionWorldEditTerrain::execute(Data *d) {
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->terrains.num);

	Terrain *t = w->terrains[index].terrain;

	// swap
	if (false) {
		std::swap(data.num_x, t->num_x);
		std::swap(data.num_z, t->num_z);
	}
	std::swap(data.pattern, t->pattern);
	std::swap(data.material_file, t->material_file);
	int num_tex_prev = t->material->textures.num;
	//t->material->copy_from(NULL, LoadMaterial(t->material_file), false);
	t->material->textures.resize(data.num_textures);
	for (int i=0;i<t->material->textures.num;i++) {
		std::swap(data.texture_file[i], t->texture_file[i]);
		std::swap(data.texture_scale[i], t->texture_scale[i]);
		try {
			t->material->textures[i] = ResourceManager::load_texture(t->texture_file[i]);
		} catch(Exception &e) {
			msg_error(e.message());
		}
	}

	// update
	/*if (old_data.NumTextures != data.NumTextures){
		delete(t->vertex_buffer);
		t->vertex_buffer = new nix::OldVertexBuffer(data.NumTextures);
	}*/
	t->force_redraw = true;

	data.num_textures = num_tex_prev;

	return NULL;
}


