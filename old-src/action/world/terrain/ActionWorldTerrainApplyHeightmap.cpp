/*
 * ActionWorldTerrainApplyHeightmap.cpp
 *
 *  Created on: 17.06.2012
 *      Author: michi
 */

#include "ActionWorldTerrainApplyHeightmap.h"
#include "../helper/ActionWorldTerrainEditHeight.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../lib/image/image.h"
#include <y/world/Terrain.h>

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

ActionWorldTerrainApplyHeightmap::ActionWorldTerrainApplyHeightmap(DataWorld *data, const Path &heightmap, float height_factor, float stretch_x, float stretch_z, const Path &filter) {
	Image im_height;
	im_height.load(heightmap);
	Image im_filter;
	if (filter != "")
		im_filter.load(filter);

	foreachi(WorldTerrain &t, data->terrains, index)
		if (t.is_selected) {
			Terrain *tt = t.terrain;
			rect dest(0, tt->num_x + 1, 0, tt->num_z + 1);
			Array<float> height;

			for (int x=dest.x1;x<dest.x2;x++)
				for (int z=dest.y1;z<dest.y2;z++) {
					float hmx = (float)x / (float)tt->num_x;
					float hmy = (float)z / (float)tt->num_z;
					float f = im_interpolate(im_height, hmx, hmy, stretch_x, stretch_z);
					if (!im_filter.is_empty())
						f *= im_interpolate(im_filter, hmx, hmy, 1, 1);
					height.add(f * height_factor);
				}

			addSubAction(new ActionWorldTerrainEditHeight(index, height, dest), data);
		}
}


