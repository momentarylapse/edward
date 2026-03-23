//
// Created by michi on 10/1/25.
//

#include "TerrainRefPanel.h"
#include "../ModeEditTerrain.h"
#include "../data/DataWorld.h"
#include <view/DocumentSession.h>
#include <view/MaterialPreviewManager.h>
#include <y/world/Terrain.h>
#include <lib/yrenderer/Material.h>
#include <lib/ygraphics/graphics-impl.h>
#include <ecs/Entity.h>
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/MaterialManager.h>
#include <storage/Storage.h>
#include <mode_material/dialog/MaterialSelector.h>



TerrainRefPanel::TerrainRefPanel(DataWorld* _data, int _index) : Node("terrain-panel") {
	from_source(R"foodelim(
Dialog terrain-panel ''
	Grid main-grid ''
		Grid ? ''
			Label ? 'Terrain' right disabled
			Grid ? ''
				Button terrain '' 'tooltip=Select a terrain'
				Button edit-terrain 'E' paddingx=5 'tooltip=Edit terrain' primary noexpandx
			---|
			Label ? 'Size' right disabled
			Label size ''
			---|
			Label ? 'Cells' right disabled
			Label cells ''
)foodelim");
	data = _data;
	index = _index;

	material_selector = new MaterialSelector(data);
	embed("main-grid", 0, 1, material_selector);

	auto e = data->entity(index);
	auto tr = e->get_component<TerrainRef>();

	event("terrain", [this, e, tr] {
		data->session->storage->file_dialog(FD_TERRAIN, false, true).then([this, e, tr] (const ComplexPath& p) {
			data->entity_edit_component(e, TerrainRef::_class, {"", "", {{"terrain", "", str(p.relative)}}});
		});
	});
	material_selector->out_selected >> create_data_sink<yrenderer::Material*>([this, e] (yrenderer::Material* m) {
		data->entity_edit_component(e, TerrainRef::_class, {"", "", {{"material", "", str(data->session->resource_manager->material_manager->get_filename(m))}}});
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
	event("edit-terrain", [this] {
		data->doc->set_mode(new ModeEditTerrain(data->doc->mode_world, index));
	});

	data->out_changed >> create_sink([this] {
		if (true) //!editing)
			update_ui();
	});
	update_ui();
}

void TerrainRefPanel::update_ui() {
	auto e = data->entity(index);
	auto tr = e->get_component<TerrainRef>();
	if (auto t = tr->terrain) {
		set_string("terrain", str(t->filename));
		set_string("size", format("%.1f x %.1f", t->pattern.x * (float)t->num_x, t->pattern.z * (float)t->num_z));
		set_string("cells", format("%d x %d", t->num_x, t->num_z));
	}
	if (auto m = tr->material) {
		material_selector->set_material(m);
	}
}
