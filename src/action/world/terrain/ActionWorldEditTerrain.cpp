/*
 * ActionWorldEditTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldEditTerrain.h"
#include "../../../EdwardWindow.h"
#include "../../../data/world/DataWorld.h"
#include "../../../y/Terrain.h"
#include "../../../y/ResourceManager.h"
#include "../../../lib/nix/nix.h"
#include "../../../lib/os/msg.h"
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
	std::swap(data.pattern, t->pattern);
	std::swap(data.material_file, t->material_file);
	int num_tex_prev = t->material->textures.num;
	//t->material->copy_from(NULL, LoadMaterial(t->material_file), false);
	t->material->textures.resize(data.num_textures);
	for (int i=0;i<t->material->textures.num;i++) {
		std::swap(data.texture_file[i], t->texture_file[i]);
		std::swap(data.texture_scale[i], t->texture_scale[i]);
		try {
			t->material->textures[i] = d->ed->resource_manager->load_texture(t->texture_file[i]);
		} catch(Exception &e) {
			msg_error(e.message());
		}
	}
	t->max.x = t->pattern.x * t->num_x; // FIXME resize...
	t->max.z = t->pattern.z * t->num_z;

	// update
	/*if (old_data.NumTextures != data.NumTextures){
		delete(t->vertex_buffer);
		t->vertex_buffer = new nix::OldVertexBuffer(data.NumTextures);
	}*/
	t->force_redraw = true;

	data.num_textures = num_tex_prev;

	return NULL;
}


