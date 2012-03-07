/*
 * DataFont.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAFONT_H_
#define DATAFONT_H_

#include "../Data.h"
#include "../../lib/x/x.h"

class DataFont: public Data
{
public:
	DataFont();
	virtual ~DataFont();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);
};

#endif /* DATAFONT_H_ */
