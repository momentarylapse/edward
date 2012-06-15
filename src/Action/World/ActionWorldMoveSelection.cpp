/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveSelection.h"
#include "../../Data/World/DataWorld.h"

ActionWorldMoveSelection::ActionWorldMoveSelection(Data *d, const vector &_pos0) :
	ActionMultiView(d, _pos0)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);

	// list of selected objects and save old pos
	foreachi(w->Object, o, i)
		if (o.is_selected){
			index.add(i);
			old_data.add(o.pos);
		}
	foreachi(w->Terrain, t, i)
		if (t.is_selected){
			terrain_index.add(i);
			terrain_old_data.add(t.pos);
		}
}

ActionWorldMoveSelection::~ActionWorldMoveSelection()
{
}

void *ActionWorldMoveSelection::execute(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(index, i, ii)
		w->Object[i].pos = old_data[ii] + param;
	foreachi(terrain_index, i, ii){
		w->Terrain[i].pos = terrain_old_data[ii] + param;
		w->Terrain[i].terrain->pos = w->Terrain[i].pos;
		w->Terrain[i].terrain->Update(-1, -1, -1, -1, TerrainUpdateVertices);
	}
	return NULL;
}



void ActionWorldMoveSelection::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(index, i, ii)
		w->Object[i].pos = old_data[ii];
	foreachi(terrain_index, i, ii){
		w->Terrain[i].pos = terrain_old_data[ii];
		w->Terrain[i].terrain->pos = w->Terrain[i].pos;
		w->Terrain[i].terrain->Update(-1, -1, -1, -1, TerrainUpdateVertices);
	}
}

