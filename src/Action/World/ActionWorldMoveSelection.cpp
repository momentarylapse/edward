/*
 * ActionWorldMoveObjects.cpp
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#include "ActionWorldMoveSelection.h"
#include "../../Data/World/DataWorld.h"

ActionWorldMoveSelection::ActionWorldMoveSelection(DataWorld *d, const vector &_param, const vector &_pos0) :
	ActionMultiView(_param, _pos0)
{
	// list of selected objects and save old pos
	foreachi(WorldObject &o, d->Objects, i)
		if (o.is_selected){
			index.add(i);
			old_data.add(o.pos);
		}
	foreachi(WorldTerrain &t, d->Terrains, i)
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
	foreachi(int i, index, ii)
		w->Objects[i].pos = old_data[ii] + param;
	foreachi(int i, terrain_index, ii){
		w->Terrains[i].pos = terrain_old_data[ii] + param;
		w->Terrains[i].terrain->pos = w->Terrains[i].pos;
		w->Terrains[i].terrain->Update(-1, -1, -1, -1, TerrainUpdateVertices);
	}
	return NULL;
}



void ActionWorldMoveSelection::undo(Data *d)
{
	DataWorld *w = dynamic_cast<DataWorld*>(d);
	foreachi(int i, index, ii)
		w->Objects[i].pos = old_data[ii];
	foreachi(int i, terrain_index, ii){
		w->Terrains[i].pos = terrain_old_data[ii];
		w->Terrains[i].terrain->pos = w->Terrains[i].pos;
		w->Terrains[i].terrain->Update(-1, -1, -1, -1, TerrainUpdateVertices);
	}
}


