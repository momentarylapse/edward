/*
 * ActionModelEditMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITMATERIAL_H_
#define ACTIONMODELEDITMATERIAL_H_

#include "../../Action.h"
#include "../../../Data/Model/DataModel.h"

class ActionModelEditMaterial : public Action {
public:
	ActionModelEditMaterial(int index, const ModelMaterial::Color &_col);
	ActionModelEditMaterial(int index, const ModelMaterial::Alpha &_a);
	string name(){ return "ModelEditMaterial"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	int mode;
	ModelMaterial::Color col;
	ModelMaterial::Alpha alpha;
};

class ActionModelMaterialAddTexture : public Action {
public:
	ActionModelMaterialAddTexture(int _index);
	string name(){ return "ModelMaterialAddTexture"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
};

class ActionModelMaterialDeleteTexture : public Action {
public:
	ActionModelMaterialDeleteTexture(int index, int level);
	string name(){ return "ModelMaterialDeleteTexture"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, level;
	ModelMaterial::TextureLevel *tl;
};


class ActionModelMaterialLoadTexture : public Action {
public:
	ActionModelMaterialLoadTexture(int index, int level, const string &filename);
	string name(){ return "ModelMaterialAddTexture"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, level;
	string filename;
};

#endif /* ACTIONMODELEDITMATERIAL_H_ */
