/*
 * DataAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef DATAADMINISTRATION_H_
#define DATAADMINISTRATION_H_

#include "../Data.h"

class AdminGameExportException
{
public:
	AdminGameExportException(const string &msg){	message = msg;	}
	string message;
};

struct GameIniData
{
	string DefScript, DefWorld, SecondWorld, DefMaterial, DefFont;
	string DefTextureFxMetal;
	void reset()
	{
		DefScript = "";
		DefWorld = "";
		SecondWorld = "";
		DefMaterial = "";
		DefFont = "";
		DefTextureFxMetal = "";
	}
	void Load(const string &dir);
	void Save(const string &dir);
};

class AdminFileList;

struct AdminFile{
	string Name;
	int Kind;
	bool Missing;
	bool Checked;
	Array<AdminFile*> Parent;
	Array<AdminFile*> Child;
	int Time;

	void add_child(AdminFile *a);
	void remove_child(AdminFile *a);
	void remove_all_children();
	void check(AdminFileList &list);
};

class AdminFileList : public Array<AdminFile*>
{
public:
	void clear();
	void clear_deep();
	void sort();
	void add_recursive(AdminFile *a);
	AdminFile *add_unchecked(int kind, const string &filename, AdminFile *source = NULL);
	AdminFile *add_unchecked_ae(int kind, const string &filename, AdminFile *source);
	AdminFile *get(int kind, const string &name);
	void remove_obsolete();

	AdminFile *add_engine_files();
	void add_from_game_ini(GameIniData &game_ini, AdminFile *f);
	void add_from_game_ini_export(AdminFileList *source, GameIniData &game_ini);
};

// for searching a file for links
struct s_admin_link
{
	string file;
	int type;
};

class DataAdministration : public Data
{
public:
	DataAdministration();
	virtual ~DataAdministration();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);

	void FraesDir(const string &root_dir, const string &dir, const string &extension);
	void MetaFraesDir(int kind);
	void TestRootDirectory();
	void SaveDatabase();
	void LoadDatabase();
	void UpdateDatabase();

	void ExportGame(const string &dir, GameIniData &game_ini);

	AdminFileList file_list;

	Array<string> cft;

	GameIniData GameIni;

	static CFile *admin_file;
};

#endif /* DATAADMINISTRATION_H_ */
