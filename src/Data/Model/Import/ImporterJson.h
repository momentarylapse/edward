/*
 * ImporterJson.h
 *
 *  Created on: 17.01.2015
 *      Author: michi
 */

#ifndef IMPORTERJSON_H_
#define IMPORTERJSON_H_

#include "../DataModel.h"

class ImporterJson
{
public:
	ImporterJson();
	virtual ~ImporterJson();

	Array<string> tokens;
	int cur_token;

	string next();
	string peek_next();
	string rnext();

	bool Import(DataModel *m, const string &filename);

	class Value
	{
	public:
		virtual ~Value();
		virtual Array<string> getKeys(){ return Array<string>(); }
		virtual string s(){ return ""; }
		virtual int i(){ return 0; }
		virtual float f(){ return 0.0f; }
		virtual int getCount(){ return 0; }
		virtual Value *get(const string &key){ return NULL; }
		virtual Value *get(int index){ return NULL; }
	};

	Value *read_value();
	Value *read_struct();
	Value *read_array();

	vector val2vec(Value *v, int offset = 0);
	quaternion val2quat(Value *v, int offset = 0);
	color val2col3(Value *v, int offset = 0);

	void importVertices(DataModel *m, Value *v);
	void importPolygons(DataModel *m, Value *v);
	void importMaterials(DataModel *m, Value *v);
	void importBones(DataModel *m, Value *v);
	void importBoneIndices(DataModel *m, Value *v, int num_influences);
	void importMoves(DataModel *m, Value *v);

	File *f;
};

#endif /* IMPORTERJSON_H_ */
