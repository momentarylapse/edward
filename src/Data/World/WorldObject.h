/*
 * WorldObject.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDOBJECT_H_
#define SRC_DATA_WORLD_WORLDOBJECT_H_

#include "../../MultiView/SingleData.h"
#include "../../lib/os/path.h"

class Object;
class ScriptInstanceData;

class WorldObject: public MultiView::SingleData {
public:
	Path filename;
	Array<ScriptInstanceData> components;
	string name;
	Object *object;
	vector ang;

	float hover_distance(MultiView::Window *win, const vec2 &m, vector &tp, float &z) override;
	bool in_rect(MultiView::Window *win, const rect &r) override;
	bool overlap_rect(MultiView::Window *win, const rect &r) override;

	void update_data();
};



#endif /* SRC_DATA_WORLD_WORLDOBJECT_H_ */
