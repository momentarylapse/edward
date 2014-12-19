/*
 * DataAdministration.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "DataAdministration.h"
#include "AdminFile.h"
#include "AdminFileList.h"
#include "GameIniData.h"
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
	GameIni = new GameIniData;
	file_list = new AdminFileList;
}

DataAdministration::~DataAdministration()
{
	delete(GameIni);
	delete(file_list);
}

void DataAdministration::FraesDir(const string &root_dir, const string &dir, const string &extension)
{
	msg_db_f("FraesDir",1);
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
}

void DataAdministration::MetaFraesDir(int kind)
{
	msg_db_f("MetaFraesDir",5);
	string extension ="x";
	cft.clear();

	string dir = ed->getRootDir(kind);
	if (kind==FD_WORLD)		extension = ".world";
	if (kind==FD_TERRAIN)	extension = ".map";
	if (kind==FD_MODEL)		extension = ".model";
	if (kind==FD_MATERIAL)	extension = ".material";
	if (kind==FD_FONT)		extension = ".xfont";
	if (kind==FD_SHADERFILE)	extension = ".glsl";
	if (kind==FD_SCRIPT)		extension = ".kaba";
	if (kind==FD_CAMERAFLIGHT)extension = ".camera";
	if (kind==FD_TEXTURE)	extension = "";
	if (kind==FD_SOUND)		extension = "";
	if (extension == "x")
		return;
	msg_db_m(("suche... (" + extension + ")\n").c_str(),5);
	FraesDir(dir, "", extension);
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

bool DataAdministration::save(const string &_filename)
{
	msg_db_f("Admin.Save",5);
	filename = _filename;
	admin_file->Create(filename);
	admin_file->WriteComment("// Number Of Files");
	admin_file->WriteInt(file_list->num);
	admin_file->WriteComment("// Files (type, filename, date, missing)");
	foreach(AdminFile *a, *file_list){
		admin_file->WriteInt(a->Kind);
		admin_file->WriteStr(a->Name);
		admin_file->WriteInt(a->Time);
		admin_file->WriteBool(a->Missing);
	}
	admin_file->WriteComment("// Links (num dests, dests...)");
	foreach(AdminFile *a, *file_list){
		admin_file->WriteInt(a->Child.num);
		foreach(AdminFile *d, a->Child){
			int n=-1;
			foreachi(AdminFile *aa, *file_list, k)
				if (d == aa){
					n=k;
					break;
				}
			admin_file->WriteInt(n);
		}
	}
	admin_file->WriteStr("#");
	admin_file->Close();
	return true;
}

void DataAdministration::SaveDatabase()
{
	save(HuiAppDirectory + "Data/admin_database.txt");
}

void DataAdministration::reset()
{
	msg_db_f("Admin.Reset",5);
	file_list->clear_deep();
}

bool DataAdministration::load(const string &_filename, bool deep)
{
	msg_db_f("Admin.Load",5);
	reset();
	filename = _filename;

	if (!admin_file->Open(filename))
		return false;
	int num = admin_file->ReadIntC();
	for (int i=0;i<num;i++){
		AdminFile *a = new AdminFile;
		file_list->add(a);
	}
	// files
	admin_file->ReadComment();
	foreach(AdminFile *a, *file_list){
		a->Kind = admin_file->ReadInt();
		a->Name = admin_file->ReadStr().sys_filename();
		a->Time = admin_file->ReadInt();
		a->Missing = admin_file->ReadBool();
		a->Checked = false;
	}
	// links
	admin_file->ReadComment();
	foreach(AdminFile *a, *file_list){
		int nd = admin_file->ReadInt();
		for (int j=0;j<nd;j++){
			int n = admin_file->ReadInt();
			a->add_child((*file_list)[n]);
		}
	}
	admin_file->Close();
	notify();
	return true;
}

void DataAdministration::LoadDatabase()
{
	load(HuiAppDirectory + "Data/admin_database.txt");
}

AdminFile *AdminFileList::add_engine_files()
{
	AdminFile *f1 = add_unchecked(-1, "x.exe");
	AdminFile *f2 = add_unchecked(-1, "config.txt");
	AdminFile *f3 = add_unchecked(-1, "game.ini");
	f1->add_child(f2);
	f1->add_child(f3);
	return f3;
}

void AdminFileList::add_from_game_ini(GameIniData &game_ini, AdminFile *f)
{
	add_unchecked_ae(FD_SCRIPT,  game_ini.DefScript, f);
	add_unchecked_ae(FD_WORLD,   game_ini.DefWorld, f);
	add_unchecked_ae(FD_WORLD,   game_ini.SecondWorld, f);
	add_unchecked_ae(FD_MATERIAL,game_ini.DefMaterial, f);
	add_unchecked_ae(FD_FONT,    game_ini.DefFont, f);
}

void AdminFileList::add_from_game_ini_export(AdminFileList *source, GameIniData &game_ini)
{
	AdminFile *a;
	if (game_ini.DefScript.num > 0){
		a = source->get(FD_SCRIPT,  game_ini.DefScript);
		if (!a)
			throw AdminGameExportException("game.ini: script file");
		add_recursive(a);
	}

	if (game_ini.DefWorld.num > 0){
		a = source->get(FD_WORLD,   game_ini.DefWorld + ".world");
		if (!a)
			throw AdminGameExportException("game.ini: initial world");
		add_recursive(a);
	}

	if (game_ini.SecondWorld.num > 0){
		a = source->get(FD_WORLD,   game_ini.SecondWorld + ".world");
		if (!a)
			throw AdminGameExportException("game.ini: second world");
		add_recursive(a);
	}

	if (game_ini.DefMaterial.num > 0){
		a = source->get(FD_MATERIAL,game_ini.DefMaterial + ".material");
		if (!a)
			throw AdminGameExportException("game.ini: default material");
		add_recursive(a);
	}

	if (game_ini.DefFont.num > 0){
		a = source->get(FD_FONT,    game_ini.DefFont + ".xfont");
		if (!a)
			throw AdminGameExportException("game.ini: default font");
		add_recursive(a);
	}
}


void DataAdministration::UpdateDatabase()
{
	msg_db_f("UpdateDatebase",0);
	ed->progress->start(_("Erstelle Datenbank"), 0);
	ed->progress->set(_("Initialisierung"), 0);

	// make sure the "Engine"-files are the first 3 ones
	AdminFile *f_game_ini = file_list->add_engine_files();

	GameIni->Load(ed->root_dir);

	// find all files
	// iterate file types
	for (int update_kind=0;update_kind<NUM_FDS;update_kind++){
		MetaFraesDir(update_kind);

		// iterate files of one type
		for (int update_index=0;(unsigned)update_index<cft.num;update_index++){
			file_list->add_unchecked(update_kind,cft[update_index].c_str(),NULL);
		}
	}

	// files in game.ini ok?
	file_list->add_from_game_ini(*GameIni, f_game_ini);




	// check all files
	for (int i=0;i<file_list->num;i++){
		(*file_list)[i]->check(*file_list);

		ed->progress->set(_("Teste Dateien"), (float)i / (float)file_list->num);
	}


	file_list->remove_obsolete();


	ed->progress->end();
	SaveDatabase();
	notify();
}

void DataAdministration::ExportGame(const string &dir, GameIniData &game_ini)
{
	if (dir == ed->root_dir)
		throw AdminGameExportException("export dir = root dir");
	AdminFileList list;
	list.add((*file_list)[0]);
	list.add((*file_list)[1]);
	list.add_from_game_ini_export(file_list, game_ini);

	ed->progress->start(_("Exportiere Spiel"), 0);
	int num_ok = 0;

	game_ini.Save(dir);

	foreachi(AdminFile *a, list, i){
		ed->progress->set((float)i / (float)list.num);
		if (a->Missing)
			continue;

		string source = ed->root_dir;
		string target = dir;
		if ((a->Kind == FD_WORLD) || (a->Kind == FD_TERRAIN)){
			source += "Maps/";
			target += "Maps/";
		}else if (a->Kind == FD_MODEL){
			source += "Objects/";
			target += "Objects/";
		}else if ((a->Kind == FD_MATERIAL) || (a->Kind == FD_SHADERFILE)){
			source += "Materials/";
			target += "Materials/";
		}else if (a->Kind == FD_FONT){
			source += "Fonts/";
			target += "Fonts/";
		}else if ((a->Kind == FD_SCRIPT) || (a->Kind == FD_CAMERAFLIGHT)){
			source += "Scripts/";
			target += "Scripts/";
		}else if (a->Kind == FD_TEXTURE){
			source += "Textures/";
			target += "Textures/";
		}
		source += a->Name;
		target += a->Name;
		if (file_copy(source, target))
			num_ok ++;
	}
	HuiInfoBox(HuiCurWindow, "info", format("%d von %d Dateien exportiern", num_ok, list.num));

	ed->progress->end();
}
