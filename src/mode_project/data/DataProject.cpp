/*
 * DataProject.cpp
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#include "DataProject.h"
#include "AdminFile.h"
#include "AdminFileList.h"
#include "GameIniData.h"
#include "../../mode_world/data/DataWorld.h"
#include "../../mode_model/data/DataModel.h"
#include "../../mode_material/data/DataMaterial.h"
//#include "../../mode_font/data/DataFont.h"
#include "../../Session.h"
#include "../../Edward.h"
//#include "../../stuff/Progress.h"
#include "../../storage/Storage.h"
#include <lib/os/app.h>
#include <lib/os/file.h>
#include <lib/os/filesystem.h>
#include <lib/os/formatter.h>

DataProject::DataProject(DocumentSession *s) :
	Data(s, -1)
{
	GameIni = new GameIniData;
	file_list = new AdminFileList;
}

DataProject::~DataProject() {
	delete GameIni;
	delete file_list;
}

void DataProject::FraesDir(const Path &root_dir, const Path &dir, const string &extension) {
	auto list = os::fs::search(root_dir | dir, "*" + extension, "fd");
	for (auto &e: list) {
		if (os::fs::is_directory(root_dir | dir | e)) {
			FraesDir(root_dir, dir | e, extension);
		} else {
			cft.add(dir | e);
		}
	}
}

void DataProject::MetaFraesDir(int kind) {
	string extension ="x";
	cft.clear();

	auto dir = session->storage->get_root_dir(kind);
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
	cft = os::fs::search(dir, "*" + extension, "fr");
	//FraesDir(dir, "", extension);
}

void DataProject::TestRootDirectory()
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

bool DataProject::save(const Path &_filename) {
	filename = _filename;
	os::fs::FileStream* f = nullptr;
	try {
		f = os::fs::open(filename, "wt");
		f->write_comment("// Number Of Files");
		f->write_int(file_list->num);
		f->write_comment("// Files (type, filename, date, missing)");
		for (AdminFile *a: *file_list) {
			f->write_int(a->Kind);
			f->write_str(a->Name.str());
			f->write_int(a->Time);
			f->write_bool(a->Missing);
		}
		f->write_comment("// Links (num dests, dests...)");
		for (AdminFile *a: *file_list) {
			f->write_int(a->Child.num);
			for (AdminFile *d: a->Child) {
				int n=-1;
				foreachi(AdminFile *aa, *file_list, k)
					if (d == aa) {
						n = k;
						break;
					}
				f->write_int(n);
			}
		}
		f->write_str("#");
		delete(f);
	} catch(...) {}
	return true;
}

void DataProject::SaveDatabase() {
	save(os::app::directory_dynamic | "admin_database.txt");
}

void DataProject::reset() {
	file_list->clear_deep();
}

bool DataProject::load(const Path &_filename, bool deep) {
	reset();
	filename = _filename;

	try {

		auto f = os::fs::open(filename, "rt");
		f->read_comment();
		int num = f->read_int();
		for (int i=0;i<num;i++) {
			AdminFile *a = new AdminFile;
			file_list->add(a);
		}
		// files
		f->read_comment();
		for (AdminFile *a: *file_list) {
			a->Kind = f->read_int();
			a->Name = f->read_str();
			a->Time = f->read_int();
			a->Missing = f->read_bool();
			a->Checked = false;
		}
		// links
		f->read_comment();
		for (AdminFile *a: *file_list) {
			int nd = f->read_int();
			for (int j=0;j<nd;j++) {
				int n = f->read_int();
				a->add_child((*file_list)[n]);
			}
		}
		delete(f);
	} catch(...) {
	}
	out_changed();
	return true;
}

void DataProject::LoadDatabase() {
	load(os::app::directory_dynamic | "admin_database.txt");
}

AdminFile *AdminFileList::add_engine_files() {
	AdminFile *f1 = add_unchecked(-1, "x.exe");
	AdminFile *f2 = add_unchecked(-1, "config.txt");
	AdminFile *f3 = add_unchecked(-1, "game.ini");
	f1->add_child(f2);
	f1->add_child(f3);
	return f3;
}

void AdminFileList::add_from_game_ini(GameIniData &game_ini, AdminFile *f) {
	add_unchecked_ae(FD_SCRIPT,  game_ini.default_script(), f);
	add_unchecked_ae(FD_WORLD,   game_ini.default_world(), f);
	add_unchecked_ae(FD_WORLD,   game_ini.second_world(), f);
	add_unchecked_ae(FD_MATERIAL,game_ini.default_material(), f);
	add_unchecked_ae(FD_FONT,    game_ini.default_font(), f);
}

void AdminFileList::add_from_game_ini_export(AdminFileList *source, GameIniData &game_ini)
{
	AdminFile *a;
	if (!game_ini.default_script().is_empty()){
		a = source->get(FD_SCRIPT, game_ini.default_script());
		if (!a)
			throw AdminGameExportException("game.ini: script file");
		add_recursive(a);
	}

	if (!game_ini.default_world().is_empty()){
		a = source->get(FD_WORLD, game_ini.default_world().with(".world"));
		if (!a)
			throw AdminGameExportException("game.ini: initial world");
		add_recursive(a);
	}

	if (!game_ini.second_world().is_empty()){
		a = source->get(FD_WORLD, game_ini.second_world().with(".world"));
		if (!a)
			throw AdminGameExportException("game.ini: second world");
		add_recursive(a);
	}

	if (!game_ini.default_material().is_empty()){
		a = source->get(FD_MATERIAL, game_ini.default_material().with(".material"));
		if (!a)
			throw AdminGameExportException("game.ini: default material");
		add_recursive(a);
	}

	if (!game_ini.default_font().is_empty()){
		a = source->get(FD_FONT, game_ini.default_font().with(".xfont"));
		if (!a)
			throw AdminGameExportException("game.ini: default font");
		add_recursive(a);
	}
}


void DataProject::UpdateDatabase() {
#if 0
	session->progress->start(_("Creating database"), 0);
	session->progress->set(_("Initializing"), 0);

	// make sure the "Engine"-files are the first 3 ones
	AdminFile *f_game_ini = file_list->add_engine_files();

	GameIni->load(session->storage->root_dir);

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
		(*file_list)[i]->check(session, *file_list);

		session->progress->set(_("Checking files"), (float)i / (float)file_list->num);
	}


	file_list->remove_obsolete();


	session->progress->end();
	SaveDatabase();
	out_changed();
#endif
}

Path kind_subdir(int kind) {
	if ((kind == FD_WORLD) || (kind == FD_TERRAIN))
		return "Maps";
	if (kind == FD_MODEL)
		return "Objects";
	if ((kind == FD_MATERIAL) || (kind == FD_SHADERFILE))
		return "Materials";
	if (kind == FD_FONT)
		return "Fonts";
	if ((kind == FD_SCRIPT) || (kind == FD_CAMERAFLIGHT))
		return "Scripts";
	if (kind == FD_TEXTURE)
		return "Textures";
	return "";
}

void DataProject::ExportGame(const Path &dir, GameIniData &game_ini) {
#if 0
	if (dir == session->storage->root_dir)
		throw AdminGameExportException("export dir = root dir");
	AdminFileList list;
	list.add((*file_list)[0]);
	list.add((*file_list)[1]);
	list.add_from_game_ini_export(file_list, game_ini);

	session->progress->start(_("Export game"), 0);
	int num_ok = 0;

	game_ini.save(dir);

	foreachi(AdminFile *a, list, i){
		session->progress->set((float)i / (float)list.num);
		if (a->Missing)
			continue;

		Path source = session->storage->root_dir | kind_subdir(a->Kind) | a->Name;
		Path target = dir | kind_subdir(a->Kind) | a->Name;
		if (FILE_OP_OK(os::fs::copy(source, target)))
			num_ok ++;
	}
	hui::info_box(hui::CurWindow, "info", format("%d von %d Dateien exportiern", num_ok, list.num));

	session->progress->end();
#endif
}
