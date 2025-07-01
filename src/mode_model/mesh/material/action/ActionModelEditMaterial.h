/*
 * ActionModelEditMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITMATERIAL_H_
#define ACTIONMODELEDITMATERIAL_H_

#include <action/Action.h>
#include "../../../data/DataModel.h"

class ActionModelEditMaterial : public Action {
public:
	ActionModelEditMaterial(int index, const ModelMaterial::Color &_col);
	string name() override { return "ModelEditMaterial"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	ModelMaterial::Color col;
};

class ActionModelMaterialAddTexture : public Action {
public:
	explicit ActionModelMaterialAddTexture(int _index);
	string name() override { return "ModelMaterialAddTexture"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
};

class ActionModelMaterialDeleteTexture : public Action {
public:
	ActionModelMaterialDeleteTexture(int index, int level);
	string name() override { return "ModelMaterialDeleteTexture"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index, level;
	ModelMaterial::TextureLevel* tl;
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
	~ActionModelMaterialScaleTexture() override;
	string name() override { return "ModelMaterialScaleTexture"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index, level;
	int width, height;
	Image *image;
};

#endif /* ACTIONMODELEDITMATERIAL_H_ */
