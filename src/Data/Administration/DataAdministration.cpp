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

DataAdministration::DataAdministration()
{
	admin_file = new CFile();
}

DataAdministration::~DataAdministration()
{
}



void DataAdministration::AddLink(AdminFile *source, AdminFile *dest)
{
	if ((!source)||(!dest))
		return;
	msg_db_r("Admin.AddLink",5);
	msg_db_m(format("link:   %s  ->  %s",source->Name.c_str(),dest->Name.c_str()).c_str(),1);

	// as source of dest
	bool add = true;
	for (int i=0;i<dest->Source.num;i++)
		if (dest->Source[i] == source)
			add = false;
	if (add)
		dest->Source.add(source);

	// as dest of source
	add = true;
	for (int i=0;i<source->Dest.num;i++)
		if (source->Dest[i] == dest)
			add = false;
	if (add)
		source->Dest.add(dest);

	msg_db_l(5);
}

void DataAdministration::RemoveLink(AdminFile *source, AdminFile *dest)
{
	if ((!source)||(!dest))	return;
	msg_db_r("Admin.RemoveLink",5);
	for (int i=dest->Source.num-1;i>=0;i--)
		if (dest->Source[i] == source)
			dest->Source.erase(i);
	for (int i=source->Dest.num-1;i>=0;i--)
		if (source->Dest[i] == dest)
			source->Dest.erase(i);
	msg_db_l(5);
}

AdminFile *DataAdministration::GetAdminFile(int kind, const string &name)
{
	string _name = SysFileName(name);
	foreach(file_list_all, a)
		if ((a->Kind == kind) && (a->Name == _name))
			return a;
	return NULL;
}

string DataAdministration::FD2Str(int k)
{
	if (k==-1)				return _("[Engine]");
	if (k==FDModel)			return _("Modell");
//	if (k==FDObject)		return _("Objekt");
//	if (k==FDItem)			return _("Item");
	if (k==FDTexture)		return _("Textur");
	if (k==FDSound)			return _("Sound");
	if (k==FDMaterial)		return _("Material");
	if (k==FDTerrain)		return _("Terrain");
	if (k==FDWorld)			return _("Welt");
	if (k==FDShaderFile)	return _("Shader");
	if (k==FDFont)			return _("Font");
	if (k==FDScript)		return _("Script");
	if (k==FDCameraFlight)	return _("Kamera");
	if (k==FDFile)			return _("Datei");
	return "???";
}
string DataAdministration::FD2Dir(int k)
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

bool DataAdministration::StringBegin(const string &buf, int start, const string &test)
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
	{"CamStartScript"	,FDCameraFlight}
};

void DataAdministration::add_possible_link(Array<s_admin_link> &l, int type, const string &filename)
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

void DataAdministration::CheckFile(AdminFile *a)
{
	if (!a)
		return;
	msg_db_r("CheckAdminFile", 5);
	msg_db_m(a->Name.c_str(), 5);


	bool really_scan = true;

	// test file existence
	admin_file->SilentFileAccess = true;
	if (admin_file->Open(FD2Dir(a->Kind) + a->Name)){

		// file ok
		int time = admin_file->GetDateModification().time;
		a->Missing = false;
		admin_file->Close();

		// different time stamp -> rescan file
		really_scan = (time != a->Time);
		if (really_scan)
			for (int j=a->Dest.num-1;j>=0;j--)
				RemoveLink(a,a->Dest[j]);
	}else{

		// no file -> missing
		a->Missing = true;
		a->Time = 0;
		really_scan = false;
		for (int j=a->Dest.num-1;j>=0;j--)
			RemoveLink(a,a->Dest[j]);
	}
	admin_file->SilentFileAccess = false;


	// rescan file?
	if (!really_scan){
		msg_db_l(5);
		return;
	}

	// find links
	Array<s_admin_link> l;
	if (a->Kind==FDWorld){
		DataWorld w;
		if (w.Load(MapDir + a->Name, false)){
			a->Time = w.file_time;
			for (int i=0;i<w.Terrain.num;i++)
				add_possible_link(l, FDTerrain, w.Terrain[i].FileName);
			for (int i=0;i<w.meta_data.SkyBoxFile.num;i++)
				add_possible_link(l, FDModel, w.meta_data.SkyBoxFile[i]);
			for (int i=0;i<w.meta_data.Script.num;i++)
				add_possible_link(l, FDScript, w.meta_data.Script[i].Filename);
			for (int i=0;i<w.Object.num;i++)
				add_possible_link(l, FDModel, w.Object[i].FileName);
		}else
			a->Missing=true;
	}else if (a->Kind==FDTerrain){
		ModeWorldTerrain t;
		if (t.Load(v0, MapDir + a->Name, false)){
			a->Time = 0; // TODO
			for (int i=0;i<t.terrain->num_textures;i++)
				add_possible_link(l, FDTexture, t.terrain->texture_file[i]);
			add_possible_link(l, FDMaterial, t.terrain->material_file);
		}else
			a->Missing=true;
	}else if (a->Kind==FDModel){
		DataModel m;
		if (m.Load(ObjectDir + a->Name,false)){
			a->Time = m.file_time;
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
		}else
			a->Missing=true;
	}else if (a->Kind==FDMaterial){
		DataMaterial m;
		if (m.Load(MaterialDir + a->Name,false)){
			a->Time = m.file_time;
			add_possible_link(l, FDShaderFile, m.Appearance.EffectFile);
			if (m.Appearance.ReflectionMode==ReflectionCubeMapStatic)
				for (int i=0;i<6;i++)
					add_possible_link(l, FDTexture, m.Appearance.ReflectionTextureFile[i]);
			for (int i=0;i<m.Appearance.NumTextureLevels;i++)
				add_possible_link(l, FDTexture, m.Appearance.TextureFile[i]);
		}else
			a->Missing=true;
	}else if (a->Kind==FDFont){
		/*DataFont f;
		if (f.Load(MaterialDir + a->Name,false)){
			a->Time = f.file_time;
			add_possible_link(l, FDTexture, f.TextureFile);
		}else*/
			a->Missing=true;
	}else if (a->Kind==FDScript){
		if (admin_file->Open(ScriptDir + a->Name)){
			a->Time=admin_file->GetDateModification().time;
			admin_file->SetBinaryMode(true);
			string buf = admin_file->ReadComplete();
			admin_file->Close();
			// would be better to compile the script and look for functions having a string constant as a parameter...
			//   -> would automatically ignore comments and   function( "aaa" + b )
			for (int i=0;i<buf.num;i++){
				for (int j=1;j<NumScriptLinks;j++)
					if (StringBegin(buf, i, ScriptLink[j].str))
						add_possible_link(l,ScriptLink[j].type,StringAfterString);
				// #include must be handled differently (relative path...)
				if (StringBegin(buf,i,ScriptLink[0].str))
					add_possible_link(l, ScriptLink[0].type, filename_no_recursion(dirname(a->Name) + StringAfterString));
			}
		}else
			a->Missing=true;
	}else{
		if (admin_file->Open(FD2Dir(a->Kind) + a->Name)){
			a->Time = admin_file->GetDateModification().time;
			admin_file->Close();
		}else
			a->Missing=true;
	}

	// recursively scan linked files
	for (int i=0;i<l.num;i++)
		AddFileUnchecked_ae(l[i].type, l[i].file,a);

	msg_db_l(5);
}

AdminFile *DataAdministration::AddFileUnchecked(int kind, const string &filename, AdminFile *source)
{
	if (filename.num <= 0)
		return NULL;
	msg_db_r("AddAdminFileUnchecked",5);
	msg_db_m(filename.c_str(),5);

	AdminFile *a = NULL;
	string _filename = SysFileName(filename);

	// is there already an entry in the database?
	foreach(file_list_all, aa)
		if ((aa->Kind == kind) && (aa->Name == _filename)){
			a = aa;
			break;
		}

	// no list entry yet -> create one
	if (!a){
		a = new AdminFile;
		file_list_all.add(a);
		a->Name = _filename;
		a->Kind = kind;
		a->Missing = false;
		a->Checked = false;
	}

	// link to meta
	AddLink(source, a);

	msg_db_l(5);
	return a;
}

// same as AddAdminFileUnchecked but used without file extensions
AdminFile *DataAdministration::AddFileUnchecked_ae(int kind, const string &filename, AdminFile *source)
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
	return AddFileUnchecked(kind, filename2, source);
}

void DataAdministration::FraesDir(const string &root_dir, const string &dir, const string &extension)
{
	msg_db_r("FraesDir",1);
	msg_db_m(dir.c_str(),1);
	Array<DirEntry> list = dir_search(root_dir + dir, "*" + extension, true);
	foreach(list, e){
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

void DataAdministration::LoadGameIni(const string &dir, GameIniData *g)
{
	msg_db_r("LoadGameIni",5);
	CFile *f = OpenFile(dir + "game.ini");
	g->DefScript = f->ReadStrC();
	g->DefWorld = f->ReadStrC();
	g->SecondWorld = f->ReadStrC();
	g->DefMaterial = f->ReadStrC();
	g->DefFont = f->ReadStrC();
	g->DefTextureFxMetal = f->ReadStrC();
	delete(f);
	msg_db_l(5);
}

void DataAdministration::SaveGameIni(const string &dir, GameIniData *g)
{
	msg_db_r("SaveGameIni",5);
	CFile *f = CreateFile(dir + "game.ini");
	f->WriteStr("// Main Script");
	f->WriteStr(g->DefScript);
	f->WriteStr("// Default World");
	f->WriteStr(g->DefWorld);
	f->WriteStr("// Second World");
	f->WriteStr(g->SecondWorld);
	f->WriteStr("// Default Material");
	f->WriteStr(g->DefMaterial);
	f->WriteStr("// Default Font");
	f->WriteStr(g->DefFont);
	f->WriteStr("// Texture FX-metal");
	f->WriteStr(g->DefTextureFxMetal);
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

void DataAdministration::SaveDatabase()
{
	msg_db_r("SaveAdminDatabase",5);
	admin_file->Create(HuiAppDirectory + "Data/admin_database.txt");
	admin_file->WriteComment("// Number Of Files");
	admin_file->WriteInt(file_list_all.num);
	admin_file->WriteComment("// Files (type, filename, date, missing)");
	foreach(file_list_all, a){
		admin_file->WriteInt(a->Kind);
		admin_file->WriteStr(a->Name);
		admin_file->WriteInt(a->Time);
		admin_file->WriteBool(a->Missing);
	}
	admin_file->WriteComment("// Links (num dests, dests...)");
	foreach(file_list_all, a){
		admin_file->WriteInt(a->Dest.num);
		foreach(a->Dest, d){
			int n=-1;
			foreachi(file_list_all, aa, k)
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
}

void DataAdministration::ResetDatabase()
{
	msg_db_r("ResetDatabase",5);
	foreach(file_list_all, a)
		delete(a);
	file_list_all.clear();
	msg_db_l(5);
}

void DataAdministration::LoadDatabase()
{
	msg_db_r("LoadDatabase",5);
	ResetDatabase();

	if (!admin_file->Open(HuiAppDirectory + "Data/admin_database.txt")){
		msg_db_l(5);
		return;
	}
	int num = admin_file->ReadIntC();
	for (int i=0;i<num;i++){
		AdminFile *a = new AdminFile;
		file_list_all.add(a);
	}
	// files
	admin_file->ReadComment();
	foreach(file_list_all, a){
		a->Kind = admin_file->ReadInt();
		a->Name = SysFileName(admin_file->ReadStr());
		a->Time = admin_file->ReadInt();
		a->Missing = admin_file->ReadBool();
		a->Checked = false;
	}
	// links
	admin_file->ReadComment();
	foreach(file_list_all, a){
		int nd = admin_file->ReadInt();
		for (int j=0;j<nd;j++){
			int n = admin_file->ReadInt();
			AddLink(a, file_list_all[n]);
		}
	}
	admin_file->Close();
	Notify("Changed");
	msg_db_l(5);
}

void DataAdministration::UpdateDatabase()
{
	msg_db_r("UpdateDatebase",0);
	ed->progress->Start(_("Erstelle Datenbank"), 0);
	ed->progress->Set(_("Initialisierung"), 0);

	// make sure the "Engine"-files are the first 3 ones
	AdminFile *f1 = AddFileUnchecked(-1, "x.exe");
	AdminFile *f2 = AddFileUnchecked(-1, "config.txt");
	AdminFile *f3 = AddFileUnchecked(-1, "game.ini");
	AddLink(f1,f2);
	AddLink(f1,f3);
	LoadGameIni(ed->RootDir,&GameIni);

	HuiGetTime(0);

	// find all files
	// iterate file types
	for (int update_kind=0;update_kind<NumFDs;update_kind++){
		MetaFraesDir(update_kind);

		// iterare files of one type
		for (int update_index=0;(unsigned)update_index<cft.num;update_index++){
			AddFileUnchecked(update_kind,cft[update_index].c_str(),NULL);
		}
	}

	// files in game.ini ok?
	AddFileUnchecked_ae(FDScript,	GameIni.DefScript,f3);
	AddFileUnchecked_ae(FDWorld,	GameIni.DefWorld,f3);
	AddFileUnchecked_ae(FDWorld,	GameIni.SecondWorld,f3);
	AddFileUnchecked_ae(FDMaterial,GameIni.DefMaterial,f3);
	AddFileUnchecked_ae(FDFont,	GameIni.DefFont,f3);
	AddFileUnchecked_ae(FDTexture,	GameIni.DefTextureFxMetal,f3);




	for (int i=0;i<file_list_all.num;i++){
		CheckFile(file_list_all[i]);

		ed->progress->Set(_("Teste Dateien"), (float)i / (float)file_list_all.num);
	}


	for (int i=file_list_all.num-1;i>=0;i--){
		// missing and unwanted -> remove
		if ((file_list_all[i]->Missing) && (file_list_all[i]->Source.num == 0))
			file_list_all.erase(i);
	}


	ed->progress->End();
	SaveDatabase();
	Notify("Changed");
	msg_db_l(0);
}

void DataAdministration::FindRecursive(AdminFileList &a, AdminFile *to_add, bool source, int levels)
{
	// already in list?
	for (int i=0;i<a.num;i++)
		if (a[i] == to_add)
			return;

	// add
	a.add(to_add);

	// recursion...
	//if (levels==0)		return;
	if (source){
		for (int j=0;j<to_add->Source.num;j++)
			FindRecursive(a, to_add->Source[j], source, levels - 1);
	}else{
		for (int j=0;j<to_add->Dest.num;j++)
			FindRecursive(a, to_add->Dest[j], source, levels - 1);
	}
}

void DataAdministration::SortList(AdminFileList &a)
{
	msg_db_r("SortList",1);
	int num = a.num;

	// sorting (by type)
	for (int i=0;i<num-1;i++)
		for (int j=i;j<num;j++)
			if (a[i]->Kind>a[j]->Kind){
				AdminFile *t=a[i];
				a[i]=a[j];
				a[j]=t;
			}
	// sorting (by name)
	for (int i=0;i<num-1;i++)
		if (a[i]->Kind>=0)
			for (int j=i;j<num;j++)
				if (a[i]->Kind==a[j]->Kind)
					if (a[i]->Name.compare(a[j]->Name) > 0){
						AdminFile *t=a[i];
						a[i]=a[j];
						a[j]=t;
					}
	msg_db_l(1);
}

