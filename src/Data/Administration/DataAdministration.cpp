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
#include "../../Storage/Storage.h"

DataAdministration::DataAdministration() :
	Data(-1)
{
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
	Array<DirEntry> list = dir_search(root_dir + dir, "*" + extension, true);
	for (DirEntry &e: list){
		if (e.is_dir){
			FraesDir(root_dir, dir + e.name + "/", extension);
		}else{
			cft.add(dir + e.name);
		}
	}
}

void DataAdministration::MetaFraesDir(int kind)
{
	string extension ="x";
	cft.clear();

	string dir = storage->get_root_dir(kind);
	if (kind==FD_WORLD)		extension = ".world";
	if (kind==FD_TERRAIN)	extension = ".map";
	if (kind==FD_MODEL)		extension = ".model";
	if (kind==FD_MATERIAL)	extension = ".material";
	if (kind==FD_FONT)		extension = ".xfont";
	if (kind==FD_SHADERFILE)	extension = ".shader";
	if (kind==FD_SCRIPT)		extension = ".kaba";
	if (kind==FD_CAMERAFLIGHT)extension = ".camera";
	if (kind==FD_TEXTURE)	extension = "";
	if (kind==FD_SOUND)		extension = "";
	if (extension == "x")
		return;
	FraesDir(dir, "", extension);
}

void DataAdministration::TestRootDirectory()
{
	/*RootDirCorrect = file_test_existence(RootDir + "game.ini");
	if (!RootDirCorrect){
		if (HuiQuestionBox(MainWin,_("Question"), format(_("Woring directory \"%s\" does not contain a file named \"game.ini\". Do you want one to be created?"),RootDir.c_str()),false)=="hui:yes"){
			GameIni.reset();
			SaveGameIni(RootDir, &GameIni);
		}
	}
	RootDirCorrect=true;
	SetRootDirectory(RootDir);*/
}

bool DataAdministration::save(const string &_filename)
{
	filename = _filename;
	File* f = NULL;
	try{
		f = FileCreateText(filename);
		f->write_comment("// Number Of Files");
		f->write_int(file_list->num);
		f->write_comment("// Files (type, filename, date, missing)");
		for (AdminFile *a: *file_list){
			f->write_int(a->Kind);
			f->write_str(a->Name);
			f->write_int(a->Time);
			f->write_bool(a->Missing);
		}
		f->write_comment("// Links (num dests, dests...)");
		for (AdminFile *a: *file_list){
			f->write_int(a->Child.num);
			for (AdminFile *d: a->Child){
				int n=-1;
				foreachi(AdminFile *aa, *file_list, k)
					if (d == aa){
						n=k;
						break;
					}
				f->write_int(n);
			}
		}
		f->write_str("#");
		delete(f);
	}catch(...){}
	return true;
}

void DataAdministration::SaveDatabase()
{
	save(app->directory + "admin_database.txt");
}

void DataAdministration::reset()
{
	file_list->clear_deep();
}

bool DataAdministration::load(const string &_filename, bool deep)
{
	reset();
	filename = _filename;

	try{

		File *f = FileOpenText(filename);
		f->read_comment();
		int num = f->read_int();
		for (int i=0;i<num;i++){
			AdminFile *a = new AdminFile;
			file_list->add(a);
		}
		// files
		f->read_comment();
		for (AdminFile *a: *file_list){
			a->Kind = f->read_int();
			a->Name = f->read_str().sys_filename();
			a->Time = f->read_int();
			a->Missing = f->read_bool();
			a->Checked = false;
		}
		// links
		f->read_comment();
		for (AdminFile *a: *file_list){
			int nd = f->read_int();
			for (int j=0;j<nd;j++){
				int n = f->read_int();
				a->add_child((*file_list)[n]);
			}
		}
		delete(f);
	}catch(...){
	}
	notify();
	return true;
}

void DataAdministration::LoadDatabase()
{
	load(app->directory + "admin_database.txt");
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
	ed->progress->start(_("Creating database"), 0);
	ed->progress->set(_("Initializing"), 0);

	// make sure the "Engine"-files are the first 3 ones
	AdminFile *f_game_ini = file_list->add_engine_files();

	GameIni->Load(storage->root_dir);

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

		ed->progress->set(_("Checking files"), (float)i / (float)file_list->num);
	}


	file_list->remove_obsolete();


	ed->progress->end();
	SaveDatabase();
	notify();
}

void DataAdministration::ExportGame(const string &dir, GameIniData &game_ini)
{
	if (dir == storage->root_dir)
		throw AdminGameExportException("export dir = root dir");
	AdminFileList list;
	list.add((*file_list)[0]);
	list.add((*file_list)[1]);
	list.add_from_game_ini_export(file_list, game_ini);

	ed->progress->start(_("Export game"), 0);
	int num_ok = 0;

	game_ini.Save(dir);

	foreachi(AdminFile *a, list, i){
		ed->progress->set((float)i / (float)list.num);
		if (a->Missing)
			continue;

		string source = storage->root_dir;
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
	hui::InfoBox(hui::CurWindow, "info", format("%d von %d Dateien exportiern", num_ok, list.num));

	ed->progress->end();
}
