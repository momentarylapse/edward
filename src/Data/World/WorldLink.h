/*
 * WorldLink.h
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#ifndef SRC_DATA_WORLD_WORLDLINK_H_
#define SRC_DATA_WORLD_WORLDLINK_H_

#include "../../MultiView/SingleData.h"


enum class LinkType {
	SOCKET,
	HINGE,
	UNIVERSAL,
	SPRING
};

string link_type_canonical(LinkType t);

class WorldLink: public MultiView::SingleData {
public:
	WorldLink();
	LinkType type;
	int object[2];
	vector ang;
	float param[4];
	float friction;
};


#endif /* SRC_DATA_WORLD_WORLDLINK_H_ */
