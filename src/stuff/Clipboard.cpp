/*
 * Clipboard.cpp
 *
 *  Created on: 15 Sept 2023
 *      Author: michi
 */

#include "Clipboard.h"

Clipboard clipboard;

Clipboard::Clipboard() {
}

void Clipboard::set_mesh_data(const Geometry& geo) {
	temp_geo = geo;
	out_changed();
}

bool Clipboard::has_mesh_data() const {
	return temp_geo.vertex.num > 0;
}

