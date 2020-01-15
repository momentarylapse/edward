/*
 * ExporterJson.h
 *
 *  Created on: 17.01.2015
 *      Author: michi
 */

#ifndef EXPORTERJSON_H_
#define EXPORTERJSON_H_

#include "../DataModel.h"

class ExporterJson
{
public:
	ExporterJson();
	virtual ~ExporterJson();

	string vecToJson(vector &v);
	string col3ToJson(color &c);
	string qToJson(quaternion &q);
	string materialToJson(ModelMaterial *m);
	string moveToJson(ModelMove &m, DataModel *model);
	string boneToJson(ModelBone &b);

	bool Export(DataModel *m, const string &filename);
};

#endif /* EXPORTERJSON_H_ */
