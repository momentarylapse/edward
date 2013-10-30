/*
 * DataMaterial.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAMATERIAL_H_
#define DATAMATERIAL_H_

#include "../Data.h"
#include "../../x/material.h"

class NixTexture;
class NixShader;

class DataMaterial: public Data
{
public:
	DataMaterial();
	virtual ~DataMaterial();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void ApplyForRendering();
	int EffectiveTextureLevels();
	void UpdateTextures();

	/*struct TextureLevel
	{
		string Filename;
		int Texture;
	};

	struct TextureData
	{
		Array<TextureLevel> level;
	};
	TextureData Texture;

	struct ColorData
	{
		color Ambient, diffuse, specular, emission;
		float shininess;
	};
	ColorData Color;

	struct TransparencyData
	{
		int Mode, Source, Destination;
		float Factor;
		bool ZBuffer;
	};
	TransparencyData Transparency;

	struct ReflectionData
	{
		int Mode, Density, Size;
		string TextureFile[6];
	};
	ReflectionData Reflection;

	struct EffectData
	{
		bool Water;
		string Filename;
		int Index;
		int ShiningDensity, ShiningLength;
	};
	EffectData Effect;*/

	struct AppearanceData
	{
		// properties
		int NumTextureLevels;
		NixTexture *Texture[MATERIAL_MAX_TEXTURES];
		string TextureFile[MATERIAL_MAX_TEXTURES];

		// color
		color ColorAmbient, ColorDiffuse, ColorSpecular, ColorEmissive;
		float ColorShininess;

		// transparency
		int TransparencyMode, AlphaSource, AlphaDestination;
		float AlphaFactor;
		bool AlphaZBuffer;

		// reflection
		int ReflectionMode, ReflectionSize;
		float ReflectionDensity;
		string ReflectionTextureFile[6];
		NixTexture *CubeMap;

		// shader
		string ShaderFile;

		void Reset();
		NixShader *GetShader() const;
	};

	AppearanceData Appearance;

	struct PhysicsData
	{
		float RCJump, RCStatic, RCSliding, RCRolling;
		float RCVJumpMin, RCVSlidingMin;
		// ...
		bool Burnable;
		float BurningTemperature, BurningIntensity;

		void Reset();
	};
	PhysicsData Physics;

	struct SoundData
	{
		int NumRules;

		void Reset();
	};
	SoundData Sound;

/*	// properties
	int NumTextureLevels;
	int Texture[MATERIAL_MAX_TEXTURES];
	string TextureFile[MATERIAL_MAX_TEXTURES];

	// appearance
	color ColorAmbient, ColorDiffuse, ColorSpecular, ColorEmissive;
	float ColorShininess;
	int ShiningDensity,ShiningLength;
	int TransparencyMode,AlphaSource,AlphaDestination;
	float AlphaFactor;
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
	int NumSoundRules;*/

	NixShader *Shader;
};

#endif /* DATAMATERIAL_H_ */
