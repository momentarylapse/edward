/*
 * ModeModelMaterial.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODEMODELMATERIAL_H_
#define MODEMODELMATERIAL_H_

#include "../../lib/x/x.h"

class ModeModelMaterial
{
public:
	ModeModelMaterial();
	virtual ~ModeModelMaterial();

	void reset();
	void CheckTextures();
	void ApplyForRendering();

	string MaterialFile;
	Material *material;
	int NumTextures;
	int Texture[MODEL_MAX_TEXTURES];
	string TextureFile[MODEL_MAX_TEXTURES];

	int TransparencyMode;
	int AlphaFactor,AlphaSource,AlphaDestination;
	bool AlphaZBuffer;
	bool UserColor;
	int Color[4][4];
	int Shininess;
};

#endif /* MODEMODELMATERIAL_H_ */
