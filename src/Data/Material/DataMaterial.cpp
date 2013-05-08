/*
 * DataMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "DataMaterial.h"
#include "../../Edward.h"
#include "../../lib/nix/nix.h"



DataMaterial::DataMaterial() :
	Data(FDMaterial)
{
	Shader = -1;
	Appearance.CubeMap = NixCreateCubeMap(128);

	Reset();
}

DataMaterial::~DataMaterial()
{
	Reset();
}

bool DataMaterial::Save(const string & _filename)
{
	filename = _filename;
	ed->MakeDirs(filename);

	CFile *f = CreateFile(filename);
	f->WriteFileFormatVersion(false, 4);

	f->WriteComment("// Textures");
	f->WriteInt(Appearance.NumTextureLevels);
	for (int i=0;i<Appearance.NumTextureLevels;i++)
		f->WriteStr(Appearance.TextureFile[i]);
	f->WriteComment("// Colors");
	write_color_argb(f, Appearance.ColorAmbient);
	write_color_argb(f, Appearance.ColorDiffuse);
	write_color_argb(f, Appearance.ColorSpecular);
	f->WriteInt(Appearance.ColorShininess);
	write_color_argb(f, Appearance.ColorEmissive);
	f->WriteComment("// Transparency");
	f->WriteInt(Appearance.TransparencyMode);
	f->WriteInt(Appearance.AlphaFactor * 100.0f);
	f->WriteInt(Appearance.AlphaSource);
	f->WriteInt(Appearance.AlphaDestination);
	f->WriteBool(Appearance.AlphaZBuffer);
	f->WriteComment("// Appearance");
	f->WriteInt(0);
	f->WriteInt(0);
	f->WriteBool(false);
	f->WriteComment("// Reflection");
	f->WriteInt(Appearance.ReflectionMode);
	f->WriteInt(Appearance.ReflectionDensity);
	f->WriteInt(Appearance.ReflectionSize);
	for (int i=0;i<6;i++)
		f->WriteStr(Appearance.ReflectionTextureFile[i]);
	f->WriteComment("// ShaderFile");
	f->WriteStr(Appearance.ShaderFile);
	f->WriteComment("// Physics");
	f->WriteInt(Physics.RCJump * 1000.0f);
	f->WriteInt(Physics.RCStatic * 1000.0f);
	f->WriteInt(Physics.RCSliding * 1000.0f);
	f->WriteInt(Physics.RCRolling * 1000.0f);
	f->WriteInt(Physics.RCVJumpMin * 1000.0f);
	f->WriteInt(Physics.RCVSlidingMin * 1000.0f);
	f->WriteComment("// Sound");
	f->WriteInt(Sound.NumRules);
	for (int i=0;i<Sound.NumRules;i++){
	}


	f->WriteStr("#");
	f->Close();
	delete(f);

	ed->SetMessage(_("Gespeichert!"));
	action_manager->MarkCurrentAsSave();
	return true;
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
	file_time = f->GetDateModification().time;

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
		read_color_argb(f, Appearance.ColorAmbient);
		read_color_argb(f, Appearance.ColorDiffuse);
		read_color_argb(f, Appearance.ColorSpecular);
		Appearance.ColorShininess = f->ReadInt();
		read_color_argb(f, Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode = f->ReadIntC();
		Appearance.AlphaFactor = (float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource = f->ReadInt();
		Appearance.AlphaDestination = f->ReadInt();
		Appearance.AlphaZBuffer = f->ReadBool();
		// Appearance
		f->ReadIntC();
		f->ReadInt();
		f->ReadBool();
		// Reflection
		Appearance.ReflectionMode = f->ReadIntC();
		Appearance.ReflectionDensity = (float)f->ReadInt();
		Appearance.ReflectionSize = f->ReadInt();
		for (int i=0;i<6;i++)
			Appearance.ReflectionTextureFile[i] = f->ReadStr();
		// ShaderFile
		Appearance.ShaderFile = f->ReadStrC();
		// Physics
		Physics.RCJump = (float)f->ReadIntC() * 0.001f;
		Physics.RCStatic = (float)f->ReadInt() * 0.001f;
		Physics.RCSliding = (float)f->ReadInt() * 0.001f;
		Physics.RCRolling = (float)f->ReadInt() * 0.001f;
		Physics.RCVJumpMin = (float)f->ReadInt() * 0.001f;
		Physics.RCVSlidingMin = (float)f->ReadInt() * 0.001f;
		if (ffv >= 4){
			// Sound
			//NumSoundRules=f->ReadIntC();
			Sound.NumRules=0;
		}

		//AlphaZBuffer=(TransparencyMode!=TransparencyModeFunctions)&&(TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==2){
		// Colors
		f->ReadComment();
		read_color_argb(f, Appearance.ColorAmbient);
		read_color_argb(f, Appearance.ColorDiffuse);
		read_color_argb(f, Appearance.ColorSpecular);
		Appearance.ColorShininess = (float)f->ReadInt();
		read_color_argb(f, Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode = f->ReadIntC();
		Appearance.AlphaFactor = (float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource = f->ReadInt();
		Appearance.AlphaDestination = f->ReadInt();
		// Appearance
		int MetalDensity = f->ReadIntC();
		if (MetalDensity > 0){
			Appearance.ReflectionMode = ReflectionMetal;
			Appearance.ReflectionDensity = (float)MetalDensity;
		}
		f->ReadInt();
		f->ReadInt();
		bool Mirror = f->ReadBool();
		if (Mirror)
			Appearance.ReflectionMode = ReflectionMirror;
		f->ReadBool();
		// ShaderFile
		string sf = f->ReadStrC();
		if (sf.num > 0)
			Appearance.ShaderFile = sf + ".fx.glsl";
		// Physics
		Physics.RCJump = (float)f->ReadIntC() * 0.001f;
		Physics.RCStatic = (float)f->ReadInt() * 0.001f;
		Physics.RCSliding = (float)f->ReadInt() * 0.001f;
		Physics.RCRolling = (float)f->ReadInt() * 0.001f;
		Physics.RCVJumpMin = (float)f->ReadInt() * 0.001f;
		Physics.RCVSlidingMin = (float)f->ReadInt() * 0.001f;

		Appearance.AlphaZBuffer=(Appearance.TransparencyMode!=TransparencyModeFunctions)&&(Appearance.TransparencyMode!=TransparencyModeFactor);
	}else if (ffv==1){
		// Colors
		f->ReadComment();
		read_color_argb(f, Appearance.ColorAmbient);
		read_color_argb(f, Appearance.ColorDiffuse);
		read_color_argb(f, Appearance.ColorSpecular);
		Appearance.ColorShininess = (float)f->ReadInt();
		read_color_argb(f, Appearance.ColorEmissive);
		// Transparency
		Appearance.TransparencyMode = f->ReadIntC();
		Appearance.AlphaFactor = (float)f->ReadInt() * 0.01f;
		Appearance.AlphaSource = f->ReadInt();
		Appearance.AlphaDestination = f->ReadInt();
		// Appearance
		int MetalDensity = f->ReadIntC();
		if (MetalDensity > 0){
			Appearance.ReflectionMode = ReflectionMetal;
			Appearance.ReflectionDensity = (float)MetalDensity;
		}
		f->ReadInt();
		f->ReadInt();
		Appearance.ReflectionMode = (f->ReadBool() ? ReflectionMirror : ReflectionNone);
		bool Mirror = f->ReadBool();
		if (Mirror)
			Appearance.ReflectionMode = ReflectionMirror;
		// ShaderFile
		string sf = f->ReadStrC();
		if (sf.num > 0)
			Appearance.ShaderFile = sf + ".fx.glsl";

		Appearance.AlphaZBuffer = (Appearance.TransparencyMode != TransparencyModeFunctions) && (Appearance.TransparencyMode != TransparencyModeFactor);
	}else{
		ed->ErrorBox(format(_("Falsches Datei-Format der Datei '%s': %d (statt %d - %d)"), filename.c_str(), ffv, 1, 4));
		error = true;
	}

	f->Close();
	delete(f);


	if (deep)
		UpdateTextures();

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


	ShaderFile.clear();
}

int DataMaterial::AppearanceData::GetShader() const
{
	return NixLoadShader(ShaderFile);
}


void DataMaterial::PhysicsData::Reset()
{
	RCJump = 0.7f;
	RCStatic = 0.8f;
	RCSliding = 0.5f;
	RCRolling = 0.2f;
	RCVJumpMin = 10;
	RCVSlidingMin = 10;
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

	if (Shader >= 0)
		NixUnrefShader(Shader);
	Shader = -1;

	Appearance.Reset();
	Physics.Reset();
	Sound.Reset();


	ResetHistory();
	Notify("Change");
}


int DataMaterial::EffectiveTextureLevels()
{
	if ((Appearance.ReflectionMode == ReflectionCubeMapStatic) || (Appearance.ReflectionMode == ReflectionCubeMapDynamical))
		return 4;
	if (Appearance.NumTextureLevels == 0)
		return 1;
	return Appearance.NumTextureLevels;
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

	NixSetShader(Shader);

	int num_tex = EffectiveTextureLevels();
	if (num_tex > 1)
		NixSetTextures(Appearance.Texture, num_tex);
	else
		NixSetTexture(Appearance.Texture[0]);
}

void DataMaterial::UpdateTextures()
{
	msg_db_r("Mat.UpdateTextures", 1);
	for (int i=0;i<MATERIAL_MAX_TEXTURES;i++)
		Appearance.Texture[i] = -1;
	for (int i=0;i<Appearance.NumTextureLevels;i++)
		Appearance.Texture[i] = NixLoadTexture(Appearance.TextureFile[i]);
	for (int i=0;i<6;i++)
		NixFillCubeMap(Appearance.CubeMap, i, NixLoadTexture(Appearance.ReflectionTextureFile[i]));
	if ((Appearance.ReflectionMode == ReflectionCubeMapStatic) || (Appearance.ReflectionMode == ReflectionCubeMapDynamical))
		Appearance.Texture[3] = Appearance.CubeMap;
	msg_db_l(1);
}




