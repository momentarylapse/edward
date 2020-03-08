/*
 * WorldLink.cpp
 *
 *  Created on: 08.03.2020
 *      Author: michi
 */

#include "WorldLink.h"


string link_type_canonical(LinkType t) {
	if (t == LinkType::SOCKET)
		return "socket";
	if (t == LinkType::HINGE)
		return "hinge";
	if (t == LinkType::SPRING)
		return "spring";
	if (t == LinkType::UNIVERSAL)
		return "universal";
	return "???";
}


WorldLink::WorldLink() {
	type = LinkType::SOCKET;
	object[0] = -1;
	object[1] = -1;
	ang = v_0;
	//param = {0,0,0,0};
	memset(param, 0, sizeof(param));
	friction = 0;
}


