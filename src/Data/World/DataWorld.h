/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include "../Data.h"
#include "../../lib/x/x.h"

class DataWorld: public Data
{
public:
	DataWorld();
	virtual ~DataWorld();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);
};

#endif /* DATAWORLD_H_ */
