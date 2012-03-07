/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include "../../lib/x/x.h"

class DataMaterial: public Data
{
public:
	DataMaterial();
	virtual ~DataMaterial();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);
};

#endif /* DATAMATERIAL_H_ */
