/*
 * AdminFile.cpp
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#include "AdminFile.h"
#include "AdminFileList.h"
#include "DataAdministration.h"
#include "../World/DataWorld.h"
#include "../Model/DataModel.h"
#include "../Material/DataMaterial.h"
#include "../Font/DataFont.h"
#include "../../Edward.h"

AdminFile::AdminFile()
{
	Kind = -1;
	Missing = false;
	Checked = false;
	Time = 0;
}

AdminFile::AdminFile(const string &filename, int kind)
{
	Name = filename;
	Kind = kind;
	Missing = false;
	Checked = false;
	Time = 0;
}

void AdminFile::add_child(AdminFile *child)
{
	if (!child)
		return;
	msg_db_r("AdminFile.add_child",5);
	msg_db_m(format("link:   %s  ->  %s",Name.c_str(),child->Name.c_str()).c_str(),1);

	// as parent of child
	bool add = true;
	for (int i=0;i<child->Parent.num;i++)
		if (child->Parent[i] == this)
			add = false;
	if (add)
		child->Parent.add(this);

	// as dest of source
	add = true;
	for (int i=0;i<Child.num;i++)
		if (Child[i] == child)
			add = false;
	if (add)
		Child.add(child);

	msg_db_l(5);
}

void AdminFile::remove_child(AdminFile *child)
{
	msg_db_r("AdminFile.remove_child",5);
	for (int i=child->Parent.num-1;i>=0;i--)
		if (child->Parent[i] == this)
			child->Parent.erase(i);
	for (int i=Child.num-1;i>=0;i--)
		if (Child[i] == child)
			Child.erase(i);
	msg_db_l(5);
}

void AdminFile::remove_all_children()
{
	for (int j=Child.num-1;j>=0;j--)
		remove_child(Child[j]);
}


static string StringAfterString;
bool StringBegin(const string &buf, int start, const string &test)
{
	// does the string <test> start here?
	for (int i=0;i<test.num;i++)
		if (buf[i+start] != test[i])
			return false;
	// skip whitespace and '(' till '"' is found...
	int b=-1;
	for (int i=0;i<256;i++){
		if (buf[start+test.num+i]=='\"'){
			b=start+test.num+i+1;
			break;
		}else if ( (buf[start+test.num+i]!=' ') && (buf[start+test.num+i]!='\t') && (buf[start+test.num+i]!='\n') && (buf[start+test.num+i]!='(') )
			break;
	}
	if (b<0)
		return false;
	// scan the actual string
	StringAfterString = "";
	for (int i=0;i<256;i++){
		if (buf[b+i]=='\"')
			break;
		StringAfterString.append_1_single(buf[b+i]);
	}
	// scan for '+'... to ignore
	for (int i=0;i<256;i++){
		char c=buf[b+StringAfterString.num+i+1];
		if (c=='+')
			return false;
		if ( (c!=' ') && (c!='\t') && (c='\n') )
			break;
	}
	return true;
}


struct sScriptLink{
	const string str;
	int type;
};

#define NumScriptLinks		13
sScriptLink ScriptLink[NumScriptLinks]={
	{"#include"			,FDScript},
	{"FileOpen"			,FDFile},
	{"LoadTexture"		,FDTexture},
	{"LoadModel"		,FDModel},
	{"CreateObject"		,FDModel},
	{"LoadWorld"		,FDWorld},
	{"LoadMaterial"		,FDMaterial},
	{"LoadShaderFile"	,FDShaderFile},
	{"SoundCreate"		,FDSound},
	{"SoundEmit"		,FDSound},
	{"MusicLoad"		,FDSound},
	{"LoadXFont"		,FDFont},
	{"StartScript"	,FDCameraFlight}
};



// for searching a file for links
struct s_admin_link
{
	string file;
	int type;
};

void add_possible_link(Array<s_admin_link> &l, int type, const string &filename)
{
	if (filename.num < 0)
		return;
	// already in list
	for (int i=0;i<l.num;i++)
		if (l[i].type == type)
			if (l[i].file == filename)
				return;
	msg_db_m(string("pos. link: " + filename).c_str(),6);
	s_admin_link link;
	link.file = filename;
	link.type = type;
	l.add(link);
}

void AdminFile::check(AdminFileList &list)
{
	msg_db_r("AdminFile.check", 5);
	msg_db_m(Name.c_str(), 5);


	bool really_scan = true;

	// test file existence
	CFile *f = DataAdministration::admin_file;
	f->SilentFileAccess = true;
	if (f->Open(ed->GetRootDir(Kind) + Name)){

		// file ok
		int _time = f->GetDateModification().time;
		Missing = false;
		f->Close();

		// different time stamp -> rescan file
		really_scan = (_time != Time);
		if (really_scan)
			remove_all_children();
	}else{

		// no file -> missing
		Missing = true;
		Time = 0;
		really_scan = false;
		if (Kind >= 0)
			remove_all_children();
	}
	f->SilentFileAccess = false;


	// rescan file?
	if (!really_scan){
		msg_db_l(5);
		return;
	}

	// find links
	Array<s_admin_link> l;
	if (Kind==FDWorld){
		DataWorld w;
		if (w.Load(MapDir + Name, false)){
			Time = w.file_time;
			for (int i=0;i<w.Terrains.num;i++)
				add_possible_link(l, FDTerrain, w.Terrains[i].FileName);
			for (int i=0;i<w.meta_data.SkyBoxFile.num;i++)
				add_possible_link(l, FDModel, w.meta_data.SkyBoxFile[i]);
			for (int i=0;i<w.meta_data.ScriptFile.num;i++)
				add_possible_link(l, FDScript, w.meta_data.ScriptFile[i]);
			for (int i=0;i<w.Objects.num;i++)
				add_possible_link(l, FDModel, w.Objects[i].FileName);
		}else
			Missing=true;
	}else if (Kind==FDTerrain){
		WorldTerrain t;
		if (t.Load(v_0, MapDir + Name, false)){
			Time = 0; // TODO
			for (int i=0;i<t.terrain->num_textures;i++)
				add_possible_link(l, FDTexture, t.terrain->texture_file[i]);
			add_possible_link(l, FDMaterial, t.terrain->material_file);
		}else
			Missing=true;
	}else if (Kind==FDModel){
		DataModel m;
		if (m.Load(ObjectDir + Name,false)){
			Time = m.file_time;
			for (int i=0;i<m.Bone.num;i++)
				add_possible_link(l, FDModel, m.Bone[i].ModelFile);
			for (int i=0;i<m.Fx.num;i++){
				if (m.Fx[i].Kind==FXKindScript)
					add_possible_link(l, FDScript, m.Fx[i].File);
				if (m.Fx[i].Kind==FXKindSound)
					add_possible_link(l, FDSound, m.Fx[i].File);
			}
			foreach(string &s, m.meta_data.Inventary)
				add_possible_link(l, FDModel, s);
			for (int i=0;i<m.Material.num;i++){
				add_possible_link(l, FDMaterial, m.Material[i].MaterialFile);
				for (int j=0;j<m.Material[i].NumTextures;j++)
					add_possible_link(l, FDTexture, m.Material[i].TextureFile[j]);
			}
			add_possible_link(l, FDScript, m.meta_data.ScriptFile);
		}else
			Missing=true;
	}else if (Kind==FDMaterial){
		DataMaterial m;
		if (m.Load(MaterialDir + Name,false)){
			Time = m.file_time;
			add_possible_link(l, FDShaderFile, m.Appearance.EffectFile);
			if (m.Appearance.ReflectionMode==ReflectionCubeMapStatic)
				for (int i=0;i<6;i++)
					add_possible_link(l, FDTexture, m.Appearance.ReflectionTextureFile[i]);
			for (int i=0;i<m.Appearance.NumTextureLevels;i++)
				add_possible_link(l, FDTexture, m.Appearance.TextureFile[i]);
		}else
			Missing=true;
	}else if (Kind==FDFont){
		DataFont f;
		if (f.Load(MaterialDir + Name,false)){
			Time = f.file_time;
			add_possible_link(l, FDTexture, f.TextureFile);
		}else
			Missing=true;
	}else if (Kind==FDScript){
		if (f->Open(ScriptDir + Name)){
			Time = f->GetDateModification().time;
			f->SetBinaryMode(true);
			string buf = f->ReadComplete();
			f->Close();
			// would be better to compile the script and look for functions having a string constant as a parameter...
			//   -> would automatically ignore comments and   function( "aaa" + b )
			for (int i=0;i<buf.num;i++){
				for (int j=1;j<NumScriptLinks;j++)
					if (StringBegin(buf, i, ScriptLink[j].str))
						add_possible_link(l,ScriptLink[j].type,StringAfterString);
				// #include must be handled differently (relative path...)
				if (StringBegin(buf,i,ScriptLink[0].str))
					add_possible_link(l, ScriptLink[0].type, (Name.dirname() + StringAfterString).no_recursion());
			}
		}else
			Missing=true;
	}else{
		if (f->Open(ed->GetRootDir(Kind) + Name)){
			Time = f->GetDateModification().time;
			f->Close();
		}else
			Missing=true;
	}

	// recursively scan linked files
	for (int i=0;i<l.num;i++)
		list.add_unchecked_ae(l[i].type, l[i].file, this);

	msg_db_l(5);
}
