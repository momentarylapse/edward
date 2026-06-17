//
// Created by Michael Ankele on 2025-04-21.
//

#include "EditTerrainPanel.h"
#include "../ModeEditTerrain.h"
#include "../action/ActionWorldResizeTerrain.h"
#include "../../data/DataWorld.h"
#include "../../data/WorldTerrain.h"
#include <lib/image/image.h>
#include <storage/Storage.h>
#include <view/dialogs/CommonDialogs.h>
#include <world/Terrain.h>
#include <y/EngineData.h>

EditTerrainPanel::EditTerrainPanel(ModeEditTerrain* mode) : Node<xhui::Panel>("edit-terrain-panel") {
	mode_terrain = mode;
	from_source(R"foodelim(
Dialog edit-terrain-panel ''
	Grid ? '' margin=7
		Label ? 'Terrain' big center
		---|
		Grid ? '' class=card
			Group ? 'Brush'
				ListView add-list 'a' nobar cangrabfocus=no noexpandy height=220
		---|
		Grid ? '' class=card
			Group ? 'Texture mapping'
				Grid ? ''
					SpinButton tex0-scale '' 'tooltip=Texture scale layer #0' range=-999:999:0.01 expandx
					SpinButton tex0-rotation '' 'tooltip=Texture rotation layer #0' range=-999:999:0.1 expandx
					---|
					SpinButton tex1-scale '' 'tooltip=Texture scale layer #1' range=-999:999:0.01
					SpinButton tex1-rotation '' 'tooltip=Texture rotation layer #1' range=-999:999:0.1
		---|
		Grid ? '' class=card
			Group ? 'Operations'
				Grid ? ''
					Button resize 'Resize...'
					---|
					Button load-heightmap 'Load heightmap...'
					---|
					Button save '' image=hui:save 'tooltip=Save'
					Button save-as '' image=hui:save-as 'tooltip=Save as'
)foodelim");

	event("resize", [this] {
		auto t = mode_terrain->terrain();
		IntPairDialog::ask(this, "New terrain size", t->num_x, t->num_z).then([this] (const base::tuple<int,int>& size) {
			mode_terrain->data->execute(new ActionWorldResizeTerrain(mode_terrain->index, size.a, size.b));
		});
	});
	event("load-heightmap", [this] {
		mode_terrain->session->storage->file_dialog(FD_TEXTURE, false, false).then([this] (const ComplexPath& path) {
			FloatDialog::ask(this, "Maximum height", 10.0f).then([this, path] (float scale) {
				auto t = mode_terrain->terrain();
				auto im = Image::load(path.complete);
				for (int i=0; i<t->num_x; i++)
					for (int j=0; j<t->num_z; j++)
						t->height[i * (t->num_z+1) + j] = im->get_pixel(i % im->width, j % im->height).brightness() * scale;
				t->update(-1, -1, -1, -1, TerrainUpdateAll);
				delete im;
			});
		});
	});
	event("save", [this] {
		auto t = mode_terrain->terrain();
		WorldTerrain ttt;
		ttt.terrain = t;
		if (mode_terrain->session->storage->save(engine.map_dir | t->filename, &ttt))
			mode_terrain->session->info("saved: " + str(t->filename));
	});

	for (int i=0; i<2; i++) {
		event(format("tex%d-scale", i), [this] { on_edit_texture_map(); });
		event(format("tex%d-rotation", i), [this] { on_edit_texture_map(); });
	}

	update_ui();

	mode_terrain->data->out_terrain_changed >> create_data_sink<const Terrain*>([this] (const Terrain*) {
		if (!user_editing)
			update_ui();
	});
}

void EditTerrainPanel::update_ui() {
	auto t = mode_terrain->terrain();
	for (int i=0; i<2; i++) {
		set_float(format("tex%d-scale", i), t->texture_scale[i].x);
		set_float(format("tex%d-rotation", i), t->texture_scale[i].y / pi * 180);
	}
}

void EditTerrainPanel::on_edit_texture_map() {
	user_editing = true;
	auto t = mode_terrain->terrain();
	vec3 texture_scale[8];
	memset(&texture_scale, 0, sizeof(texture_scale));
	for (int i=0; i<2; i++) {
		texture_scale[i].x = get_float(format("tex%d-scale", i));
		texture_scale[i].y = get_float(format("tex%d-rotation", i)) * pi / 180;
	}
	mode_terrain->data->edit_terrain_meta_data(t, t->pattern, texture_scale);
	user_editing = false;
}

