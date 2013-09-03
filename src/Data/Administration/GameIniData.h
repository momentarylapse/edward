/*
 * GameIniData.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef GAMEINIDATA_H_
#define GAMEINIDATA_H_

#include "../Data.h"


class GameIniData
{
public:
	string DefScript, DefWorld, SecondWorld, DefMaterial, DefFont;
	void reset();
	void Load(const string &dir);
	void Save(const string &dir);
};


#endif /* GAMEINIDATA_H_ */
