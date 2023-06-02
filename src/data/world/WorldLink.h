/*
 * WorldLink.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDLINK_H_
#define SRC_DATA_WORLD_WORLDLINK_H_

#include "../../multiview/SingleData.h"

class ScriptInstanceData;

enum class LinkType {
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING
};

string link_type_canonical(LinkType t);

class WorldLink: public MultiView::SingleData {
public:
	LinkType type = LinkType::SOCKET;
	int object[2] = {-1,-1};
	vec3 ang = vec3::ZERO;
	float param[4] = {0,0,0,0};
	float friction = 0;
	Array<ScriptInstanceData> components;
};


#endif /* SRC_DATA_WORLD_WORLDLINK_H_ */
