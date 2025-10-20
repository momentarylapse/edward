//
// Created by michi on 10/1/25.
//

#include "TerrainRefPanel.h"
#include "../ModeEditTerrain.h"
#include "../data/DataWorld.h"
#include <view/DocumentSession.h>
#include <y/world/Terrain.h>
#include <ecs/Entity.h>

TerrainRefPanel::TerrainRefPanel(DataWorld* _data, int _index) : Panel("terrain-panel") {
	from_source(R"foodelim(
Dialog terrain-panel ''
	Grid ? ''
		Grid ? ''
			Label ? 'Terrain' right disabled
			Button filename ''
			---|
			Label ? 'Size' right disabled
			Label size ''
			---|
			Label ? 'Cells' right disabled
			Label cells ''
		---|
		Grid ? ''
			Label ? '' expandx
			Button edit 'Edit' primary noexpandx
)foodelim");
	data = _data;
	index = _index;
	auto e = data->entity(index);
	auto tr = e->get_component<TerrainRef>();
	set_string("filename", str(tr->filename));
	if (auto t = tr->terrain) {
		set_string("size", format("%.1f x %.1f", t->pattern.x * (float)t->num_x, t->pattern.z * (float)t->num_z));
		set_string("cells", format("%d x %d", t->num_x, t->num_z));
	}
	/*event("size-x", [this] {
		auto e = data->entity(index);
		if (auto t = e->get_component<TerrainRef>()->terrain)
			data->edit_terrain_meta_data(index, {get_float("size-x") / (float)t->num_x, 0, t->pattern.z});
	});
	event("size-z", [this] {
		auto e = data->entity(index);
		if (auto t = e->get_component<TerrainRef>()->terrain)
			data->edit_terrain_meta_data(index, {t->pattern.x, 0, get_float("size-z") / (float)t->num_z});
	});*/
	event("edit", [this] {
		data->doc->set_mode(new ModeEditTerrain(data->doc->mode_world, index));
	});
}
