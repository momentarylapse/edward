/*
 * ActionModelEditMaterial.h
 *
 *  Created on: 06.06.2012
 *      Author: michi
 */

#ifndef ACTIONMODELEDITMATERIAL_H_
#define ACTIONMODELEDITMATERIAL_H_

#include <lib/history/MergableAction.h>
#include "../../../data/DataModel.h"

class Image;
class Session;

class ActionModelEditMaterial : public history::MergableAction {
public:
	ActionModelEditMaterial(Session* session, yrenderer::Material* material, const yrenderer::Material& new_material);
	string name() const override { return "ModelEditMaterial"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

	bool absorb(Action *previous) override;

private:
	Session* session;
	yrenderer::Material* material;
	yrenderer::Material new_material;
};

class ActionModelMaterialAddTexture : public history::Action {
public:
	explicit ActionModelMaterialAddTexture(int _index, shared<ygfx::Texture> texture);
	string name() const override { return "ModelMaterialAddTexture"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	shared<ygfx::Texture> texture;
};

class ActionModelMaterialDeleteTexture : public history::Action {
public:
	ActionModelMaterialDeleteTexture(int index, int level);
	string name() const override { return "ModelMaterialDeleteTexture"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index, level;
	shared<ygfx::Texture> texture;
};


class ActionModelMaterialScaleTexture : public history::Action {
public:
	ActionModelMaterialScaleTexture(int index, int level, int width, int height);
	~ActionModelMaterialScaleTexture() override;
	string name() const override { return "ModelMaterialScaleTexture"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index, level;
	int width, height;
	Image *image;
};

#endif /* ACTIONMODELEDITMATERIAL_H_ */
