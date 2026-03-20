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

class Image;

class ActionModelEditMaterial : public Action {
public:
	ActionModelEditMaterial(yrenderer::Material* material, const yrenderer::Material& new_material);
	string name() override { return "ModelEditMaterial"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	yrenderer::Material* material;
	yrenderer::Material new_material;
};

class ActionModelMaterialAddTexture : public Action {
public:
	explicit ActionModelMaterialAddTexture(int _index, shared<ygfx::Texture> texture);
	string name() override { return "ModelMaterialAddTexture"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index;
	shared<ygfx::Texture> texture;
};

class ActionModelMaterialDeleteTexture : public Action {
public:
	ActionModelMaterialDeleteTexture(int index, int level);
	string name() override { return "ModelMaterialDeleteTexture"; }

	void *execute(Data *d) override;
	void undo(Data *d) override;

private:
	int index, level;
	shared<ygfx::Texture> texture;
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
