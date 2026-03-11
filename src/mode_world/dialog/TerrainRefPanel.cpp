//
// Created by michi on 10/1/25.
//

#include "TerrainRefPanel.h"
#include "../ModeEditTerrain.h"
#include "../data/DataWorld.h"
#include <view/DocumentSession.h>
#include <y/world/Terrain.h>
#include <lib/yrenderer/Material.h>
#include <lib/ygraphics/graphics-impl.h>
#include <ecs/Entity.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <storage/Storage.h>

TerrainRefPanel::TerrainRefPanel(DataWorld* _data, int _index) : Panel("terrain-panel") {
	from_source(R"foodelim(
Dialog terrain-panel ''
	Grid ? ''
		Grid ? ''
			Label ? 'Terrain' right disabled
			Button terrain ''
			---|
			Label ? 'Size' right disabled
			Label size ''
			---|
			Label ? 'Cells' right disabled
			Label cells ''
			---|
			Label ? 'Material' right disabled
			Button material ''
		---|
		Grid ? ''
			Label ? '' expandx
			Button edit 'Edit' primary noexpandx
)foodelim");
	data = _data;
	index = _index;
	auto e = data->entity(index);
	auto tr = e->get_component<TerrainRef>();
	if (auto t = tr->terrain) {
		set_string("terrain", str(t->filename));
		set_string("size", format("%.1f x %.1f", t->pattern.x * (float)t->num_x, t->pattern.z * (float)t->num_z));
		set_string("cells", format("%d x %d", t->num_x, t->num_z));
	}
	if (auto m = tr->material) {
		set_string("material", str(data->session->ctx->material_manager->get_filename(m)));
	}

	event("terrain", [this, tr] {
		data->session->storage->file_dialog(FD_TERRAIN, false, true).then([this, tr] (const ComplexPath& p) {
			tr->terrain = data->session->resource_manager->load_terrain_lazy(p.relative);
			tr->terrain->reload(data->session->resource_manager);
			set_string("terrain", str(p.relative));
		});
	});
	event("material", [this, tr] {
		data->session->storage->file_dialog(FD_MATERIAL, false, true).then([this, tr] (const ComplexPath& p) {
			tr->material = data->session->resource_manager->load_material(p.relative.no_ext());
			set_string("material", str(p.relative.no_ext()));
		});
	});
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
