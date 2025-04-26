//
// Created by Michael Ankele on 2025-04-21.
//

#include "EditTerrainPanel.h"
#include "../ModeEditTerrain.h"
#include "../action/terrain/ActionWorldResizeTerrain.h"
#include "../data/DataWorld.h"
#include <lib/image/image.h>
#include <storage/Storage.h>
#include <view/dialogs/CommonDialogs.h>
#include <world/Terrain.h>

EditTerrainPanel::EditTerrainPanel(ModeEditTerrain* mode) : Node<xhui::Panel>("edit-terrain-panel") {
	mode_terrain = mode;
	from_source(R"foodelim(
Dialog edit-terrain-panel ''
	Grid ? ''
		Label ? 'Terrain' big center
		---|
		Grid ? '' class=card
			Group ? 'Brush'
				ListView add-list 'a' nobar cangrabfocus=no noexpandy height=220
		---|
		Grid ? '' class=card
			Group ? 'Operations'
				Grid ? ''
					Button resize 'Resize...'
					---|
					Button load-heightmap 'Load heightmap...'
)foodelim");
	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
	min_width_user = 320;

	event("resize", [this] {
		auto& t = mode_terrain->terrain();
		IntPairDialog::ask(this, "New terrain size", t.terrain->num_x, t.terrain->num_z).then([this] (const base::tuple<int,int>& size) {
			mode_terrain->data->execute(new ActionWorldResizeTerrain(mode_terrain->index, size.a, size.b));
		});
	});
	event("load-heightmap", [this] {
		mode_terrain->session->storage->file_dialog(FD_TEXTURE, false, false).then([this] (const ComplexPath& path) {
			FloatDialog::ask(this, "Maximum height", 10.0f).then([this, path] (float scale) {
				auto& t = mode_terrain->terrain();
				auto im = Image::load(path.complete);
				for (int i=0; i<t.terrain->num_x; i++)
					for (int j=0; j<t.terrain->num_z; j++)
						t.terrain->height[i * (t.terrain->num_z+1) + j] = im->get_pixel(i % im->width, j % im->height).brightness() * scale;
				t.terrain->update(-1, -1, -1, -1, TerrainUpdateAll);
				delete im;
			});
		});
	});
}

