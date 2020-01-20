/*
 * FormatModelJson.h
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#ifndef SRC_STORAGE_FORMAT_FORMATMODELJSON_H_
#define SRC_STORAGE_FORMAT_FORMATMODELJSON_H_

#include "Format.h"
#include "../../Data/Model/DataModel.h"

class DataModel;
class File;

class FormatModelJson : public TypedFormat<DataModel> {
public:
	FormatModelJson();

	void _load(const string &filename, DataModel *data, bool deep) override;
	void _save(const string &filename, DataModel *data) override;




	Array<string> tokens;
	int cur_token;

	string next();
	string peek_next();
	string rnext();

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

#endif /* SRC_STORAGE_FORMAT_FORMATMODELJSON_H_ */
