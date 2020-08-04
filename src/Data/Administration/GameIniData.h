/*
 * GameIniData.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef GAMEINIDATA_H_
#define GAMEINIDATA_H_

#include "../Data.h"


class GameIniData {
public:
	Path default_script, default_world, second_world, default_material, default_font;
	void reset();
	void load(const Path &dir);
	void save(const Path &dir);
};


#endif /* GAMEINIDATA_H_ */
