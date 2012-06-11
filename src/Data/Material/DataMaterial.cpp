/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "../../Edward.h"
#include "../../lib/nix/nix.h"



DataMaterial::DataMaterial()
{
	Reset();
}

DataMaterial::~DataMaterial()
{
}

bool DataMaterial::Save(const string & _filename)
{
	return false;
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
			Appearance.NumTextureLevels = f->ReadIntC();
			for (int i=0;i<Appearance.NumTextureLevels;i++)
				Appearance.TextureFile[i] = f->ReadStr();
			if ((Appearance.NumTextureLevels == 1) && (Appearance.TextureFile[0].num == 0)){
				Appearance.NumTextureLevels = 0;
			}
		}
		// Colors
		f->ReadComment();
		read_color_argb(f,Appearance.ColorAmbient);
		read_color_argb(f,Appearance.ColorDiffuse);
		read_color_argb(f,Appearance.ColorSpecular);
		Appearance.ColorShininess=f->ReadInt();
		read_color_argb(f,Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode=f->ReadIntC();
		Appearance.AlphaFactor=(float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource=f->ReadInt();
		Appearance.AlphaDestination=f->ReadInt();
		Appearance.AlphaZBuffer=f->ReadBool();
		// Appearance
		Appearance.ShiningDensity=(float)f->ReadIntC();
		Appearance.ShiningLength=(float)f->ReadInt();
		Appearance.Water=f->ReadBool();
		// Reflection
		Appearance.ReflectionMode=f->ReadIntC();
		Appearance.ReflectionDensity=(float)f->ReadInt();
		Appearance.ReflectionSize=f->ReadInt();
		for (int i=0;i<6;i++)
			Appearance.ReflectionTextureFile[i] = f->ReadStr();
		// ShaderFile
		Appearance.EffectFile = f->ReadStrC();
		// Physics
		Physics.RCJump=f->ReadIntC();
		Physics.RCStatic=f->ReadInt();
		Physics.RCSliding=f->ReadInt();
		Physics.RCRolling=f->ReadInt();
		Physics.RCVJumpMin=f->ReadInt();
		Physics.RCVSlidingMin=f->ReadInt();
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->ReadIntC();
			Sound.NumRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->ReadComment();
		read_color_argb(f,Appearance.ColorAmbient);
		read_color_argb(f,Appearance.ColorDiffuse);
		read_color_argb(f,Appearance.ColorSpecular);
		Appearance.ColorShininess=(float)f->ReadInt();
		read_color_argb(f,Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode=f->ReadIntC();
		Appearance.AlphaFactor=(float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource=f->ReadInt();
		Appearance.AlphaDestination=f->ReadInt();
		// Appearance
		int MetalDensity=f->ReadIntC();
		if (MetalDensity>0){
			Appearance.ReflectionMode=ReflectionMetal;
			Appearance.ReflectionDensity=(float)MetalDensity;
		}
		Appearance.ShiningDensity=(float)f->ReadInt();
		Appearance.ShiningLength=(float)f->ReadInt();
		bool Mirror=f->ReadBool();
		if (Mirror)
			Appearance.ReflectionMode=ReflectionMirror;
		Appearance.Water=f->ReadBool();
		// ShaderFile
		Appearance.EffectFile = f->ReadStrC();
		// Physics
		Physics.RCJump=f->ReadIntC();
		Physics.RCStatic=f->ReadInt();
		Physics.RCSliding=f->ReadInt();
		Physics.RCRolling=f->ReadInt();
		Physics.RCVJumpMin=f->ReadInt();
		Physics.RCVSlidingMin=f->ReadInt();

		Appearance.AlphaZBuffer=(Appearance.TransparencyMode!=TransparencyModeFunctions)&&(Appearance.TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==1){
		// Colors
		f->ReadComment();
		read_color_argb(f,Appearance.ColorAmbient);
		read_color_argb(f,Appearance.ColorDiffuse);
		read_color_argb(f,Appearance.ColorSpecular);
		Appearance.ColorShininess=(float)f->ReadInt();
		read_color_argb(f,Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode=f->ReadIntC();
		Appearance.AlphaFactor=(float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource=f->ReadInt();
		Appearance.AlphaDestination=f->ReadInt();
		// Appearance
		int MetalDensity=f->ReadIntC();
		if (MetalDensity>0){
			Appearance.ReflectionMode=ReflectionMetal;
			Appearance.ReflectionDensity=(float)MetalDensity;
		}
		Appearance.ShiningDensity=(float)f->ReadInt();
		Appearance.ShiningLength=(float)f->ReadInt();
		Appearance.ReflectionMode=(f->ReadBool()?ReflectionMirror:ReflectionNone);
		bool Mirror=f->ReadBool();
		if (Mirror)
			Appearance.ReflectionMode=ReflectionMirror;
		// ShaderFile
		Appearance.EffectFile = f->ReadStrC();

		Appearance.AlphaZBuffer=(Appearance.TransparencyMode!=TransparencyModeFunctions)&&(Appearance.TransparencyMode!=TransparencyModeFactor);
	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 4));
		error=true;
	}

	f->Close();
	delete(f);



	NixDeleteShader(Appearance.EffectIndex);
	if (deep){
		if (Appearance.EffectFile.num>0)
			Appearance.EffectIndex = NixLoadShader(MaterialDir + Appearance.EffectFile + ".fx");
		for (int i=0;i<Appearance.NumTextureLevels;i++)
			Appearance.Texture[i] = NixLoadTexture(Appearance.TextureFile[i]);
	}

	ResetHistory();
	Notify("Change");
	return !error;
}


void DataMaterial::AppearanceData::Reset()
{
	NumTextureLevels = 0;
	//Texture[0] = -1;
	//strcpy(TextureFile[0], "");

	ColorAmbient = White;
	ColorDiffuse = White;
	ColorSpecular = Black;
	ColorShininess = 20;
	ColorEmissive = Black;

	TransparencyMode = TransparencyModeNone;
	AlphaSource = AlphaDestination = 0;
	AlphaFactor = 0.5f;
	AlphaZBuffer = true;

	ReflectionMode = ReflectionNone;
	ReflectionDensity = 0;
	ReflectionSize = 128;
	for (int i=0;i<6;i++)
		ReflectionTextureFile[i] = "";

	Water = false;
	ShiningDensity = 0;
	ShiningLength = 0;
	EffectFile = "";
	EffectIndex = -1;
}


void DataMaterial::PhysicsData::Reset()
{
	RCJump = 700;
	RCStatic = 800;
	RCSliding = 500;
	RCRolling = 200;
	RCVJumpMin = 10000;
	RCVSlidingMin = 10000;
	Burnable = false;
	BurningTemperature = 500;
	BurningIntensity = 40;
}


void DataMaterial::SoundData::Reset()
{
	NumRules = 0;
}

void DataMaterial::Reset()
{

	filename = "";

	Appearance.Reset();
	Physics.Reset();
	Sound.Reset();


	ResetHistory();
	Notify("Change");
}

void DataMaterial::ApplyForRendering()
{
	NixSetMaterial(Appearance.ColorAmbient, Appearance.ColorDiffuse, Appearance.ColorSpecular, Appearance.ColorShininess, Appearance.ColorEmissive);

	NixSetAlpha(AlphaNone);
	NixSetZ(true, true);
	if (Appearance.TransparencyMode == TransparencyModeColorKeyHard){
		NixSetAlpha(AlphaColorKeyHard);
	}else if (Appearance.TransparencyMode == TransparencyModeColorKeySmooth){
		NixSetAlpha(AlphaColorKeySmooth);
	}else if (Appearance.TransparencyMode == TransparencyModeFunctions){
		NixSetAlpha(Appearance.AlphaSource, Appearance.AlphaDestination);
		NixSetZ(false, false);
	}else if (Appearance.TransparencyMode == TransparencyModeFactor){
		NixSetAlpha(Appearance.AlphaFactor);
		NixSetZ(false, false);
	}

	NixSetShader(Appearance.EffectIndex);
}




