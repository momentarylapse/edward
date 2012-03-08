/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include "../../lib/x/x.h"

#define MATERIAL_MAX_TEXTURE_LEVELS		8

class DataMaterial: public Data
{
public:
	DataMaterial();
	virtual ~DataMaterial();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void ApplyForRendering();


	// properties
	int NumTextureLevels;
	int Texture[MATERIAL_MAX_TEXTURE_LEVELS];
	string TextureFile[MATERIAL_MAX_TEXTURE_LEVELS];

	// appearance
	color ColorAmbient, ColorDiffuse, ColorSpecular, ColorEmissive;
	float ColorShininess;
	int ShiningDensity,ShiningLength;
	int TransparencyMode,AlphaFactor,AlphaSource,AlphaDestination;
	bool AlphaZBuffer;
	int ReflectionMode,ReflectionDensity,ReflectionSize;
	string ReflectionTextureFile[6];
	bool Water;
	string EffectFile;
	int EffectIndex;

	// physics
	int RCJump,RCStatic,RCSliding,RCRolling;
	int RCVJumpMin,RCVSlidingMin;
	// ...
	bool Burnable;
	int BurningTemperature,BurningIntensity;

	// sounds
	int NumSoundRules;
};

#endif /* DATAMATERIAL_H_ */
