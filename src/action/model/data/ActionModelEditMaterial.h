/*
 * ActionModelEditMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITMATERIAL_H_
#define ACTIONMODELEDITMATERIAL_H_

#include "../../Action.h"
#include "../../../data/model/DataModel.h"

class ActionModelEditMaterial : public Action {
public:
	ActionModelEditMaterial(int index, const ModelMaterial::Color &_col);
	string name(){ return "ModelEditMaterial"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index;
	ModelMaterial::Color col;
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
	ModelMaterial::TextureLevel tl;
};


class ActionModelMaterialLoadTexture : public Action {
public:
	ActionModelMaterialLoadTexture(int index, int level, const Path &filename);
	string name(){ return "ModelMaterialAddTexture"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, level;
	Path filename;
};


class ActionModelMaterialScaleTexture : public Action {
public:
	ActionModelMaterialScaleTexture(int index, int level, int width, int height);
	~ActionModelMaterialScaleTexture();
	string name(){ return "ModelMaterialScaleTexture"; }

	void *execute(Data *d);
	void undo(Data *d);

private:
	int index, level;
	int width, height;
	Image *image;
};

#endif /* ACTIONMODELEDITMATERIAL_H_ */
