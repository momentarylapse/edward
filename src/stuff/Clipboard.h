/*
 * Clipboard.h
 *
 *  Created on: 15 Sept 2023
 *      Author: michi
 */

#ifndef SRC_STUFF_CLIPBOARD_H_
#define SRC_STUFF_CLIPBOARD_H_

#include "../lib/pattern/Observable.h"
#include "../data/model/geometry/Geometry.h"

class Clipboard : public obs::Node<VirtualBase> {
public:
	Clipboard();

	bool has_mesh_data() const;
	void set_mesh_data(const Geometry& geo);

	Geometry temp_geo;
};

extern Clipboard clipboard;

#endif /* SRC_STUFF_CLIPBOARD_H_ */
