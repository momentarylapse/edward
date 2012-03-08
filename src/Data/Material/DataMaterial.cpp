/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "../../Edward.h"
#include "../../lib/nix/nix.h"


static void write_color(CFile *f, const color &c)
{
	f->WriteInt((int)(c.r * 255.0f));
	f->WriteInt((int)(c.g * 255.0f));
	f->WriteInt((int)(c.b * 255.0f));
	f->WriteInt((int)(c.a * 255.0f));
}

static void read_color(CFile *f, color &c)
{
	c.r = (float)f->ReadInt() / 255.0f;
	c.g = (float)f->ReadInt() / 255.0f;
	c.b = (float)f->ReadInt() / 255.0f;
	c.a = (float)f->ReadInt() / 255.0f;
}

DataMaterial::DataMaterial()
{
	Reset();
}

DataMaterial::~DataMaterial()
{
}

bool DataMaterial::Save(const string & _filename)
{
}



bool DataMaterial::Load(const string & _filename, bool deep)
{
	bool error=false;
	int ffv;
	Reset();

	filename = _filename;
	ed->MakeDirs(filename);
	CFile *f=new CFile();
	if (!f->Open(filename)){
		delete(f);
		ed->SetMessage(_("Kann Material-Datei nicht &offnen"));
		return false;
	}
	file_time = f->GetDate(FileDateModification).time;

	ffv=f->ReadFileFormatVersion();
	if (ffv<0){
		ed->ErrorBox(_("Datei-Format nicht ladbar!!"));
		error=true;
	}else if ((ffv == 3) || (ffv == 4)){
		if (ffv >= 4){
			NumTextureLevels = f->ReadIntC();
			for (int i=0;i<NumTextureLevels;i++)
				TextureFile[i] = f->ReadStr();
			if ((NumTextureLevels == 1) && (TextureFile[0].num == 0)){
				NumTextureLevels = 0;
			}
		}
		// Colors
		f->ReadComment();
		read_color(f,ColorAmbient);
		read_color(f,ColorDiffuse);
		read_color(f,ColorSpecular);
		ColorShininess=f->ReadInt();
		read_color(f,ColorEmissive);
		// Transparency
		TransparencyMode=f->ReadIntC();
		AlphaFactor=f->ReadInt();
		AlphaSource=f->ReadInt();
		AlphaDestination=f->ReadInt();
		AlphaZBuffer=f->ReadBool();
		// Appearance
		ShiningDensity=f->ReadIntC();
		ShiningLength=f->ReadInt();
		Water=f->ReadBool();
		// Reflection
		ReflectionMode=f->ReadIntC();
		ReflectionDensity=f->ReadInt();
		ReflectionSize=f->ReadInt();
		for (int i=0;i<6;i++)
			ReflectionTextureFile[i] = f->ReadStr();
		// ShaderFile
		EffectFile = f->ReadStrC();
		// Physics
		RCJump=f->ReadIntC();
		RCStatic=f->ReadInt();
		RCSliding=f->ReadInt();
		RCRolling=f->ReadInt();
		RCVJumpMin=f->ReadInt();
		RCVSlidingMin=f->ReadInt();
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->ReadIntC();
			NumSoundRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->ReadComment();
		read_color(f,ColorAmbient);
		read_color(f,ColorDiffuse);
		read_color(f,ColorSpecular);
		ColorShininess=f->ReadInt();
		read_color(f,ColorEmissive);
		// Transparency
		TransparencyMode=f->ReadIntC();
		AlphaFactor=f->ReadInt();
		AlphaSource=f->ReadInt();
		AlphaDestination=f->ReadInt();
		// Appearance
		int MetalDensity=f->ReadIntC();
		if (MetalDensity>0){
			ReflectionMode=ReflectionMetal;
			ReflectionDensity=MetalDensity;
		}
		ShiningDensity=f->ReadInt();
		ShiningLength=f->ReadInt();
		bool Mirror=f->ReadBool();
		if (Mirror)
			ReflectionMode=ReflectionMirror;
		Water=f->ReadBool();
		// ShaderFile
		EffectFile = f->ReadStrC();
		// Physics
		RCJump=f->ReadIntC();
		RCStatic=f->ReadInt();
		RCSliding=f->ReadInt();
		RCRolling=f->ReadInt();
		RCVJumpMin=f->ReadInt();
		RCVSlidingMin=f->ReadInt();

		AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==1){
		// Colors
		f->ReadComment();
		read_color(f,ColorAmbient);
		read_color(f,ColorDiffuse);
		read_color(f,ColorSpecular);
		ColorShininess=f->ReadInt();
		read_color(f,ColorEmissive);
		// Transparency
		TransparencyMode=f->ReadIntC();
		AlphaFactor=f->ReadInt();
		AlphaSource=f->ReadInt();
		AlphaDestination=f->ReadInt();
		// Appearance
		int MetalDensity=f->ReadIntC();
		if (MetalDensity>0){
			ReflectionMode=ReflectionMetal;
			ReflectionDensity=MetalDensity;
		}
		ShiningDensity=f->ReadInt();
		ShiningLength=f->ReadInt();
		ReflectionMode=(f->ReadBool()?ReflectionMirror:ReflectionNone);
		bool Mirror=f->ReadBool();
		if (Mirror)
			ReflectionMode=ReflectionMirror;
		// ShaderFile
		EffectFile = f->ReadStrC();

		AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);
	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 4));
		error=true;
	}

	f->Close();
	delete(f);



	NixDeleteShader(EffectIndex);
	if (deep){
		if (EffectFile.num>0)
			EffectIndex = NixLoadShader(MaterialDir + EffectFile + ".fx");
		for (int i=0;i<NumTextureLevels;i++)
			Texture[i] = NixLoadTexture(TextureFile[i]);
	}

	ResetHistory();
	return !error;
}



void DataMaterial::Reset()
{

	filename = "";

	NumTextureLevels = 0;
	//Texture[0] = -1;
	//strcpy(TextureFile[0], "");

	ColorAmbient = White;
	ColorDiffuse = White;
	ColorSpecular = Black;
	ColorShininess = 0;
	ColorEmissive = Black;
	ShiningDensity=0;
	ShiningLength=0;
	TransparencyMode=TransparencyModeNone;
	AlphaSource=AlphaDestination=0;
	AlphaFactor=50;
	AlphaZBuffer=true;
	RCJump=700;
	RCStatic=800;
	RCSliding=500;
	RCRolling=200;
	RCVJumpMin=10000;
	RCVSlidingMin=10000;
	Burnable=false;
	BurningTemperature=500;
	BurningIntensity=40;
	Water=false;
	ReflectionMode=ReflectionNone;
	ReflectionDensity=0;
	ReflectionSize=128;
	for (int i=0;i<6;i++)
		ReflectionTextureFile[i] = "";

	NumSoundRules=0;


	EffectFile = "";
	EffectIndex=-1;

	ResetHistory();
}

void DataMaterial::ApplyForRendering()
{
	color am = ColorAmbient;
	NixSetMaterial(ColorAmbient, ColorDiffuse, ColorSpecular,(float)ColorShininess, ColorEmissive);

	NixSetAlpha(AlphaNone);
	NixSetZ(true,true);
	if (TransparencyMode==TransparencyModeColorKeyHard)
		NixSetAlpha(AlphaColorKeyHard);
	if (TransparencyMode==TransparencyModeColorKeySmooth)
		NixSetAlpha(AlphaColorKeySmooth);
	if (TransparencyMode==TransparencyModeFunctions){
		NixSetAlpha(AlphaSource,AlphaDestination);
		NixSetZ(false,false);
	}
	if (TransparencyMode==TransparencyModeFactor){
		NixSetAlpha((float)AlphaFactor*0.01f);
		NixSetZ(false,false);
	}

	NixSetShader(EffectIndex);
}




