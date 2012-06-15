/*
 * ActionWorldEditTerrain.cpp
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#include "ActionWorldEditTerrain.h"
#include <assert.h>

ActionWorldEditTerrain::ActionWorldEditTerrain(int _index, const ModeWorldEditingTerrain &_data)
{
	index = _index;
	data = _data;
}

ActionWorldEditTerrain::~ActionWorldEditTerrain()
{
}

void ActionWorldEditTerrain::undo(Data *d)
{
	execute(d);
}



void *ActionWorldEditTerrain::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	assert(index >= 0);
	assert(index < w->Terrain.num);

	CTerrain *t = w->Terrain[index].terrain;

	// swap
	ModeWorldEditingTerrain old_data;
	old_data.NumX = t->num_x;
	old_data.NumZ = t->num_z;
	old_data.Pattern = t->pattern;
	old_data.MaterialFile = t->material_file;
	old_data.NumTextures = t->num_textures;
	for (int i=0;i<t->num_textures;i++){
		old_data.TextureFile[i] = t->texture_file[i];
		old_data.TextureScale[i] = t->texture_scale[i];
	}
	/*t->num_x = data.NumX;
	t->num_z = data.NumZ;
	t->pattern = data.Pattern;*/
	t->material_file = data.MaterialFile;
	t->material = MetaLoadMaterial(t->material_file);
	t->num_textures = data.NumTextures;
	for (int i=0;i<t->num_textures;i++){
		t->texture_file[i] = data.TextureFile[i];
		t->texture[i] = NixLoadTexture(t->texture_file[i]);
		t->texture_scale[i] = data.TextureScale[i];
	}
	for (int i=0;i<t->material->num_textures;i++)
		if (t->texture[i] < 0)
			t->texture[i] = t->material->texture[i];

	// update
	if (old_data.NumTextures != data.NumTextures){
		NixDeleteVB(t->vertex_buffer);
		if (data.NumTextures > 1)
			t->vertex_buffer = NixCreateVBM(65536, data.NumTextures);
		else
			t->vertex_buffer = NixCreateVB(65536);
	}
	t->force_redraw = true;

	data = old_data;

	return NULL;
}


