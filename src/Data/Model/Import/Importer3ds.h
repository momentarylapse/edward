/*
 * Importer3ds.h
 *
 *  Created on: 07.03.2013
 *      Author: michi
 */

#ifndef IMPORTER3DS_H_
#define IMPORTER3DS_H_

#include "../DataModel.h"

class Importer3ds
{
public:
	Importer3ds();
	virtual ~Importer3ds();

	bool Import(DataModel *m, const string &filename);

	void LoadMesh(DataModel *m, File *f, int length);
};

#endif /* IMPORTER3DS_H_ */
