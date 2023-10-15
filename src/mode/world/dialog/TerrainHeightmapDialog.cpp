/*
 * TerrainHeightmapDialog.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "TerrainHeightmapDialog.h"
#include "../../../action/world/terrain/ActionWorldTerrainApplyHeightmap.h"
#include "../../../storage/Storage.h"
#include "../../../lib/math/rect.h"
#include "../../../lib/math/vec2.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"

TerrainHeightmapDialog::TerrainHeightmapDialog(bool _allow_parent, DataWorld *_data) :
	hui::Dialog("terrain_heightmap_dialog", 400, 300, _data->session->win, _allow_parent)
{
	from_resource("terrain_heightmap_dialog");
	data = _data;

	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("apply", [=]{ apply_data(); });
	event("ok", [=]{ on_ok(); });

	event("height_image_find", [=]{ on_find_heightmap(); });
	event("stretch_x", [=]{ on_size_change(); });
	event("stretch_z", [=]{ on_size_change(); });
	event("filter_image_find", [=]{ on_find_filter(); });
	event_xp("preview", "hui:draw", [=](Painter *p){ on_preview_draw(p); });

	enable("ok", false);

	stretch_x = 1;
	stretch_z = 1;
	load_data();
}

void TerrainHeightmapDialog::apply_data()
{
}



void TerrainHeightmapDialog::on_size_change() {
	stretch_x = get_float("stretch_x");
	stretch_z = get_float("stretch_z");
	redraw("preview");
}



void TerrainHeightmapDialog::on_find_filter() {
	data->session->storage->file_dialog(FD_TEXTURE, false, false).then([this] (const auto& p) {
		filter_file = p.complete;
		set_string("filter_image", p.relative.str());
		filter.load(filter_file);
		redraw("preview");
	});
}





void TerrainHeightmapDialog::on_find_heightmap() {
	data->session->storage->file_dialog(FD_TEXTURE, false, false).then([this] (const auto& p) {
		heightmap_file = p.complete;
		set_string("height_image", p.relative.str());
		heightmap.load(heightmap_file);
		redraw("preview");
		enable("ok", true);
	});
}


static float c2f(const color &c) {
	return (c.r + c.g + c.b) / 3.0f;
}

// texture interpolation (without repeating the last half pixel)
static float im_interpolate(const Image &im, float x, float y, float stretch_x, float stretch_y) {
	stretch_x *= im.width;
	stretch_y *= im.height;
	x = clamp(x * stretch_x, 0.5f, stretch_x - 0.5f);
	y = clamp(y * stretch_y, 0.5f, stretch_y - 0.5f);
	return c2f(im.get_pixel_interpolated(x, y));
}

void TerrainHeightmapDialog::on_preview_draw(Painter *c) {
	if (heightmap.is_empty()) {
		c->set_color(Black);
		c->draw_rect(c->area());
	} else {
		Image m;
		int w = c->width, h = c->height;
		m.create(w, h, White);
		for (int x=0;x<w;x++)
			for (int y=0;y<h;y++) {
				float hmx = (float)x / (float)w;
				float hmy = (float)y / (float)h;
				float f = im_interpolate(heightmap, hmx, hmy, stretch_x, stretch_z);
				if (!filter.is_empty())
					f *= im_interpolate(filter, hmx, hmy, 1, 1);
				m.set_pixel(x, y, color(1, f, f, f));
			}
		c->draw_image({0, 0}, &m);
	}
}



void TerrainHeightmapDialog::on_ok() {
	float height_factor = get_float("height_factor");
	bool additive = is_checked("height_op:add");
	data->execute(new ActionWorldTerrainApplyHeightmap(data, heightmap_file, height_factor, stretch_x, stretch_z, filter_file));//, additive));
	request_destroy();
}



void TerrainHeightmapDialog::on_close() {
	request_destroy();
}



void TerrainHeightmapDialog::load_data() {
	set_float("stretch_x", stretch_x);
	set_float("stretch_z", stretch_z);
	set_float("height_factor", 100);
	check("height_op:set", true);
}


