/*
 * DataAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "DataAdministration.h"
#include "../World/DataWorld.h"
#include "../Model/DataModel.h"
#include "../Material/DataMaterial.h"
#include "../Font/DataFont.h"
#include "../../Edward.h"

CFile *DataAdministration::admin_file = NULL;

DataAdministration::DataAdministration() :
	Data(-1)
{
	if (!admin_file)
		admin_file = new CFile();
}

DataAdministration::~DataAdministration()
{
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

/*void DataAdministration::AddLink(AdminFile *source, AdminFile *dest)
{
	if ((!source) || (!dest))
		return;
	source->add_child(dest);
}

void DataAdministration::RemoveLink(AdminFile *source, AdminFile *dest)
{
	if ((!source) || (!dest))
		return;
	source->remove_child(dest);
}*/

AdminFile *AdminFileList::get(int kind, const string &name)
{
	string _name = name.sys_filename();
	foreach(AdminFile *a, *this)
		if ((a->Kind == kind) && (a->Name == _name))
			return a;
	return NULL;
}

string FD2Dir(int k)
{
	if (k==-1)				return ed->RootDir;
	if (k==FDModel)			return ObjectDir;
//	if (k==FDObject)		return ObjectDir;
//	if (k==FDItem)			return ObjectDir;
	if (k==FDTexture)		return NixTextureDir;
	if (k==FDSound)			return SoundDir;
	if (k==FDMaterial)		return MaterialDir;
	if (k==FDTerrain)		return MapDir;
	if (k==FDWorld)			return MapDir;
	if (k==FDShaderFile)	return MaterialDir;
	if (k==FDFont)			return MaterialDir;
	if (k==FDScript)		return ScriptDir;
	if (k==FDCameraFlight)	return ScriptDir;
	if (k==FDFile)			return ed->RootDir;
	return ed->RootDir;
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
	if (f->Open(FD2Dir(Kind) + Name)){

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
			for (int i=0;i<w.Terrain.num;i++)
				add_possible_link(l, FDTerrain, w.Terrain[i].FileName);
			for (int i=0;i<w.meta_data.SkyBoxFile.num;i++)
				add_possible_link(l, FDModel, w.meta_data.SkyBoxFile[i]);
			for (int i=0;i<w.meta_data.ScriptFile.num;i++)
				add_possible_link(l, FDScript, w.meta_data.ScriptFile[i]);
			for (int i=0;i<w.Object.num;i++)
				add_possible_link(l, FDModel, w.Object[i].FileName);
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
		if (f->Open(FD2Dir(Kind) + Name)){
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

AdminFile *AdminFileList::add_unchecked(int kind, const string &filename, AdminFile *source)
{
	if (filename.num <= 0)
		return NULL;
	msg_db_r("AddAdminFileUnchecked",5);
	msg_db_m(filename.c_str(),5);

	AdminFile *a = NULL;
	string _filename = filename.sys_filename();

	// is there already an entry in the database?
	foreach(AdminFile *aa, *this)
		if ((aa->Kind == kind) && (aa->Name == _filename)){
			a = aa;
			break;
		}

	// no list entry yet -> create one
	if (!a){
		a = new AdminFile;
		add(a);
		a->Name = _filename;
		a->Kind = kind;
		a->Missing = false;
		a->Checked = false;
	}

	// link to meta
	if (source)
		source->add_child(a);

	msg_db_l(5);
	return a;
}

// same as AddAdminFileUnchecked but used without file extensions
AdminFile *AdminFileList::add_unchecked_ae(int kind, const string &filename, AdminFile *source)
{
	if (filename.num<=0)
		return NULL;
	string filename2 = filename;
	if (kind==FDWorld)		filename2 += ".world";
	if (kind==FDTerrain)	filename2 += ".map";
//	if (kind==FDObject)		filename2 += ".object";
//	if (kind==FDItem)		filename2 += ".item";
	if (kind==FDModel)		filename2 += ".model";
	if (kind==FDMaterial)	filename2 += ".material";
	if (kind==FDFont)		filename2 += ".xfont";
	if (kind==FDCameraFlight)filename2 += ".camera";
	if (kind==FDShaderFile)	filename2 += ".fx";
	return add_unchecked(kind, filename2, source);
}

void AdminFileList::remove_obsolete()
{
	foreachib(AdminFile *a, *this, i){
		// missing and unwanted -> remove
		if ((a->Missing) && (a->Parent.num == 0))
			if (a->Kind >= 0) // don't remove engine files...
				erase(i);
	}
}

void AdminFileList::add_recursive(AdminFile *to_add)
{
	// already in list?
	for (int i=0;i<num;i++)
		if ((*this)[i] == to_add)
			return;

	// add
	add(to_add);

	// recursion...
	foreach(AdminFile *a, to_add->Child)
		add_recursive(a);
}

void AdminFileList::sort()
{
	msg_db_r("AdminFileList.sort",1);

	// sorting (by type)
	for (int i=0;i<num-1;i++)
		for (int j=i;j<num;j++)
			if ((*this)[i]->Kind > (*this)[j]->Kind)
				swap(i, j);
	// sorting (by name)
	for (int i=0;i<num-1;i++)
		if ((*this)[i]->Kind>=0)
			for (int j=i;j<num;j++)
				if ((*this)[i]->Kind == (*this)[j]->Kind)
					if ((*this)[i]->Name.compare((*this)[j]->Name) > 0)
						swap(i, j);
	msg_db_l(1);
}

void AdminFileList::clear()
{
	Array<AdminFile*>::clear();
}

void AdminFileList::clear_deep()
{
	foreach(AdminFile *a, *this)
		delete(a);
	Array<AdminFile*>::clear();
}

void DataAdministration::FraesDir(const string &root_dir, const string &dir, const string &extension)
{
	msg_db_r("FraesDir",1);
	msg_db_m(dir.c_str(),1);
	Array<DirEntry> list = dir_search(root_dir + dir, "*" + extension, true);
	foreach(DirEntry &e, list){
		if (e.is_dir){
			FraesDir(root_dir, dir + e.name + "/", extension);
		}else{
			msg_db_m(format("%d - %s", cft.num + 1, e.name.c_str()).c_str(),1);
			cft.add(dir + e.name);
		}
	}
	msg_db_l(1);
}

void DataAdministration::MetaFraesDir(int kind)
{
	msg_db_r("MetaFraesDir",5);
	string extension ="x";
	cft.clear();

	string dir = FD2Dir(kind);
	if (kind==FDWorld)		extension = ".world";
	if (kind==FDTerrain)	extension = ".map";
	if (kind==FDModel)		extension = ".model";
//	if (kind==FDObject)		extension = ".object";
//	if (kind==FDItem)		extension = ".item";
	if (kind==FDMaterial)	extension = ".material";
	if (kind==FDFont)		extension = ".xfont";
	if (kind==FDShaderFile)	extension = ".fx";
	if (kind==FDScript)		extension = ".kaba";
	if (kind==FDCameraFlight)extension = ".camera";
	if (kind==FDTexture)	extension = "";
	if (kind==FDSound)		extension = "";
	if (extension == "x"){
		msg_db_l(5);
		return;
	}
	msg_db_m(("suche... (" + extension + ")\n").c_str(),5);
	FraesDir(dir, "", extension);
	msg_db_l(5);
}

void GameIniData::Load(const string &dir)
{
	msg_db_r("LoadGameIni",5);
	CFile *f = OpenFile(dir + "game.ini");
	DefScript = f->ReadStrC();
	DefWorld = f->ReadStrC();
	SecondWorld = f->ReadStrC();
	DefMaterial = f->ReadStrC();
	DefFont = f->ReadStrC();
	DefTextureFxMetal = f->ReadStrC();
	delete(f);
	msg_db_l(5);
}

void GameIniData::Save(const string &dir)
{
	msg_db_r("SaveGameIni",5);
	CFile *f = CreateFile(dir + "game.ini");
	f->WriteStr("// Main Script");
	f->WriteStr(DefScript);
	f->WriteStr("// Default World");
	f->WriteStr(DefWorld);
	f->WriteStr("// Second World");
	f->WriteStr(SecondWorld);
	f->WriteStr("// Default Material");
	f->WriteStr(DefMaterial);
	f->WriteStr("// Default Font");
	f->WriteStr(DefFont);
	f->WriteStr("// Texture FX-metal");
	f->WriteStr(DefTextureFxMetal);
	f->WriteStr("#");
	delete(f);
	msg_db_l(5);
}

void DataAdministration::TestRootDirectory()
{
	/*RootDirCorrect = file_test_existence(RootDir + "game.ini");
	if (!RootDirCorrect){
		if (HuiQuestionBox(MainWin,_("Frage"), format(_("Arbeitsverzeichnis \"%s\" enth&alt keine \"game.ini\"-Datei. Soll diese erstellt werden?"),RootDir.c_str()),false)=="hui:yes"){
			GameIni.reset();
			SaveGameIni(RootDir, &GameIni);
		}
	}
	RootDirCorrect=true;
	SetRootDirectory(RootDir);*/
}

bool DataAdministration::Save(const string &_filename)
{
	msg_db_r("Admin.Save",5);
	filename = _filename;
	admin_file->Create(filename);
	admin_file->WriteComment("// Number Of Files");
	admin_file->WriteInt(file_list.num);
	admin_file->WriteComment("// Files (type, filename, date, missing)");
	foreach(AdminFile *a, file_list){
		admin_file->WriteInt(a->Kind);
		admin_file->WriteStr(a->Name);
		admin_file->WriteInt(a->Time);
		admin_file->WriteBool(a->Missing);
	}
	admin_file->WriteComment("// Links (num dests, dests...)");
	foreach(AdminFile *a, file_list){
		admin_file->WriteInt(a->Child.num);
		foreach(AdminFile *d, a->Child){
			int n=-1;
			foreachi(AdminFile *aa, file_list, k)
				if (d == aa){
					n=k;
					break;
				}
			admin_file->WriteInt(n);
		}
	}
	admin_file->WriteStr("#");
	admin_file->Close();
	msg_db_l(5);
	return true;
}

void DataAdministration::SaveDatabase()
{
	Save(HuiAppDirectory + "Data/admin_database.txt");
}

void DataAdministration::Reset()
{
	msg_db_r("Admin.Reset",5);
	file_list.clear_deep();
	msg_db_l(5);
}

bool DataAdministration::Load(const string &_filename, bool deep)
{
	msg_db_r("Admin.Load",5);
	Reset();
	filename = _filename;

	if (!admin_file->Open(filename)){
		msg_db_l(5);
		return false;
	}
	int num = admin_file->ReadIntC();
	for (int i=0;i<num;i++){
		AdminFile *a = new AdminFile;
		file_list.add(a);
	}
	// files
	admin_file->ReadComment();
	foreach(AdminFile *a, file_list){
		a->Kind = admin_file->ReadInt();
		a->Name = admin_file->ReadStr().sys_filename();
		a->Time = admin_file->ReadInt();
		a->Missing = admin_file->ReadBool();
		a->Checked = false;
	}
	// links
	admin_file->ReadComment();
	foreach(AdminFile *a, file_list){
		int nd = admin_file->ReadInt();
		for (int j=0;j<nd;j++){
			int n = admin_file->ReadInt();
			a->add_child(file_list[n]);
		}
	}
	admin_file->Close();
	Notify("Changed");
	msg_db_l(5);
	return true;
}

void DataAdministration::LoadDatabase()
{
	Load(HuiAppDirectory + "Data/admin_database.txt");
}

void DataAdministration::UpdateDatabase()
{
	msg_db_r("UpdateDatebase",0);
	ed->progress->Start(_("Erstelle Datenbank"), 0);
	ed->progress->Set(_("Initialisierung"), 0);

	// make sure the "Engine"-files are the first 3 ones
	AdminFile *f1 = file_list.add_unchecked(-1, "x.exe");
	AdminFile *f2 = file_list.add_unchecked(-1, "config.txt");
	AdminFile *f3 = file_list.add_unchecked(-1, "game.ini");
	f1->add_child(f2);
	f1->add_child(f3);
	GameIni.Load(ed->RootDir);

	// find all files
	// iterate file types
	for (int update_kind=0;update_kind<NumFDs;update_kind++){
		MetaFraesDir(update_kind);

		// iterate files of one type
		for (int update_index=0;(unsigned)update_index<cft.num;update_index++){
			file_list.add_unchecked(update_kind,cft[update_index].c_str(),NULL);
		}
	}

	// files in game.ini ok?
	file_list.add_unchecked_ae(FDScript,	GameIni.DefScript,f3);
	file_list.add_unchecked_ae(FDWorld,	GameIni.DefWorld,f3);
	file_list.add_unchecked_ae(FDWorld,	GameIni.SecondWorld,f3);
	file_list.add_unchecked_ae(FDMaterial,GameIni.DefMaterial,f3);
	file_list.add_unchecked_ae(FDFont,	GameIni.DefFont,f3);
	file_list.add_unchecked_ae(FDTexture,	GameIni.DefTextureFxMetal,f3);




	// check all files
	for (int i=0;i<file_list.num;i++){
		file_list[i]->check(file_list);

		ed->progress->Set(_("Teste Dateien"), (float)i / (float)file_list.num);
	}


	file_list.remove_obsolete();


	ed->progress->End();
	SaveDatabase();
	Notify("Changed");
	msg_db_l(0);
}

