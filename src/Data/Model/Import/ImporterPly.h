/*
 * ImporterPly.h
 *
 *  Created on: Jun 10, 2015
 *      Author: ankele
 */

#ifndef SRC_DATA_MODEL_IMPORT_IMPORTERPLY_H_
#define SRC_DATA_MODEL_IMPORT_IMPORTERPLY_H_

#include "../DataModel.h"

class ImporterPly
{
public:
	ImporterPly();
	virtual ~ImporterPly();

	bool Import(DataModel *m, const string &filename);

	File *f;
};

#endif /* SRC_DATA_MODEL_IMPORT_IMPORTERPLY_H_ */
