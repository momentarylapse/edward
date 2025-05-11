/*
 * WorldObject.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDOBJECT_H_
#define SRC_DATA_WORLD_WORLDOBJECT_H_

#include <view/SingleData.h>
#include <lib/os/path.h>

class Model;
class ScriptInstanceData;

struct WorldObject: multiview::SingleData {
	Path filename;
	string name;
	Model *object = nullptr;

	/*float hover_distance(multiview::Window *win, const vec2 &m, vec3 &tp, float &z) override;
	bool in_rect(multiview::Window *win, const rect &r) override;
	bool overlap_rect(multiview::Window *win, const rect &r) override;*/

	void update_data();
};



#endif /* SRC_DATA_WORLD_WORLDOBJECT_H_ */
