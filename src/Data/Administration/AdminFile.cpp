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
#include "../World/WorldObject.h"
#include "../World/WorldTerrain.h"
#include "../Model/DataModel.h"
#include "../Material/DataMaterial.h"
#include "../Font/DataFont.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../y/EngineData.h"
#include "../../y/Terrain.h"
#include "../../lib/kaba/kaba.h"
#include "../../lib/os/file.h"
#include "../../lib/os/filesystem.h"
#include "../../lib/os/date.h"
#include "../../y/ModelManager.h"

AdminFile::AdminFile() {
	Kind = -1;
	Missing = false;
	Checked = false;
	Time = 0;
}

AdminFile::AdminFile(const Path &filename, int kind) : AdminFile() {
	Name = filename;
	Kind = kind;
}

void AdminFile::add_child(AdminFile *child)
{
	if (!child)
		return;

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
}

void AdminFile::remove_child(AdminFile *child)
{
	for (int i=child->Parent.num-1;i>=0;i--)
		if (child->Parent[i] == this)
			child->Parent.erase(i);
	for (int i=Child.num-1;i>=0;i--)
		if (Child[i] == child)
			Child.erase(i);
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
	{"#include"			,FD_SCRIPT},
	{"FileOpen"			,FD_FILE},
	{"LoadTexture"		,FD_TEXTURE},
	{"LoadModel"		,FD_MODEL},
	{"CreateObject"		,FD_MODEL},
	{"LoadWorld"		,FD_WORLD},
	{"LoadMaterial"		,FD_MATERIAL},
	{"NixLoadShader"	,FD_SHADERFILE},
	{"SoundCreate"		,FD_SOUND},
	{"SoundEmit"		,FD_SOUND},
	{"MusicLoad"		,FD_SOUND},
	{"LoadFont"			,FD_FONT},
	{"StartScript"		,FD_CAMERAFLIGHT}
};



// for searching a file for links
struct s_admin_link
{
	Path file;
	int type;
};

void add_possible_link(Array<s_admin_link> &l, int type, const Path &filename)
{
	if (filename.is_empty())
		return;
	// already in list
	for (int i=0;i<l.num;i++)
		if (l[i].type == type)
			if (l[i].file == filename)
				return;
	s_admin_link link;
	link.file = filename;
	link.type = type;
	l.add(link);
}

void AdminFile::check(AdminFileList &list)
{
	bool really_scan = true;

	// test file existence
	try {
		// file ok
		int _time = os::fs::mtime(storage->get_root_dir(Kind) | Name).time;
		Missing = false;

		// different time stamp -> rescan file
		really_scan = (_time != Time);
		if (really_scan)
			remove_all_children();
	} catch(...) {

		// no file -> missing
		Missing = true;
		Time = 0;
		really_scan = false;
		if (Kind >= 0)
			remove_all_children();
	}


	// rescan file?
	if (!really_scan)
		return;

	// find links
	Array<s_admin_link> l;
	if (Kind==FD_WORLD){
		DataWorld w;
		if (storage->load(engine.map_dir | Name, &w, false)){
			Time = w.file_time;
			for (int i=0;i<w.terrains.num;i++)
				add_possible_link(l, FD_TERRAIN, w.terrains[i].filename);
			for (int i=0;i<w.meta_data.skybox_files.num;i++)
				add_possible_link(l, FD_MODEL, w.meta_data.skybox_files[i]);
			for (int i=0;i<w.meta_data.scripts.num;i++)
				add_possible_link(l, FD_SCRIPT, w.meta_data.scripts[i].filename);
			for (int i=0;i<w.objects.num;i++)
				add_possible_link(l, FD_MODEL, w.objects[i].filename);
		}else
			Missing=true;
	}else if (Kind==FD_TERRAIN){
		WorldTerrain t;
		if (t.load(engine.map_dir | Name, false)){
			Time = 0; // TODO
			for (int i=0;i<t.terrain->material->textures.num;i++)
				add_possible_link(l, FD_TEXTURE, t.terrain->texture_file[i]);
			add_possible_link(l, FD_MATERIAL, t.terrain->material_file);
		}else
			Missing=true;
	}else if (Kind==FD_MODEL){
		DataModel m;
		if (storage->load(engine.object_dir | Name, &m, false)){
			Time = m.file_time;
			for (int i=0;i<m.bone.num;i++)
				add_possible_link(l, FD_MODEL, m.bone[i].model_file);
			for (int i=0;i<m.fx.num;i++){
				if (m.fx[i].type == FX_TYPE_SCRIPT)
					add_possible_link(l, FD_SCRIPT, m.fx[i].file);
				if (m.fx[i].type == FX_TYPE_SOUND)
					add_possible_link(l, FD_SOUND, m.fx[i].file);
			}
			for (Path &s: m.meta_data.inventary)
				add_possible_link(l, FD_MODEL, s);
			for (auto mat: m.material){
				add_possible_link(l, FD_MATERIAL, mat->filename);
				for (auto tl: mat->texture_levels)
					add_possible_link(l, FD_TEXTURE, tl->filename);
			}
			add_possible_link(l, FD_SCRIPT, m.meta_data.script_file);
		}else
			Missing=true;
	}else if (Kind==FD_MATERIAL){
		DataMaterial m;
		if (storage->load(engine.material_dir | Name, &m, false)){
			Time = m.file_time;
			add_possible_link(l, FD_SHADERFILE, m.shader.file);
			if (m.appearance.reflection_mode == ReflectionMode::CUBE_MAP_STATIC)
				for (int i=0;i<6;i++)
					add_possible_link(l, FD_TEXTURE, m.appearance.reflection_texture_file[i]);
			for (Path &tf: m.appearance.texture_files)
				add_possible_link(l, FD_TEXTURE, tf);
		}else
			Missing=true;
	}else if (Kind==FD_FONT){
		DataFont f;
		if (storage->load(engine.font_dir | Name, &f, false)){
			Time = f.file_time;
			add_possible_link(l, FD_TEXTURE, f.global.TextureFile);
		}else
			Missing=true;
	}else if (Kind==FD_SCRIPT){
		try{
			Time = os::fs::mtime(kaba::config.directory | Name).time;
			string buf = os::fs::read_text(kaba::config.directory | Name);
			// would be better to compile the script and look for functions having a string constant as a parameter...
			//   -> would automatically ignore comments and   function( "aaa" + b )
			for (int i=0;i<buf.num;i++){
				for (int j=1;j<NumScriptLinks;j++)
					if (StringBegin(buf, i, ScriptLink[j].str))
						add_possible_link(l,ScriptLink[j].type,StringAfterString);
				// #include must be handled differently (relative path...)
				if (StringBegin(buf,i,ScriptLink[0].str))
					add_possible_link(l, ScriptLink[0].type, (Name.parent() | StringAfterString).canonical());
			}
		}catch(...){
			Missing=true;
		}
	} else {
		try {
			Time = os::fs::mtime(storage->get_root_dir(Kind) | Name).time;
		} catch(...) {
			Missing=true;
		}
	}

	// recursively scan linked files
	for (int i=0;i<l.num;i++)
		list.add_unchecked_ae(l[i].type, l[i].file, this);
}
