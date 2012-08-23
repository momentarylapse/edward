/*
 * DataAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef DATAADMINISTRATION_H_
#define DATAADMINISTRATION_H_

#include "../Data.h"

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
	void sort();
	void add_recursive(AdminFile *a);
	AdminFile *add_unchecked(int kind, const string &filename, AdminFile *source = NULL);
	AdminFile *add_unchecked_ae(int kind, const string &filename, AdminFile *source);
	AdminFile *get(int kind, const string &name);
	void remove_obsolete();
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

	void Reset(){}
	bool Load(const string &_filename, bool deep = true){	return false;	}
	bool Save(const string &_filename){	return false;	}

	//void AddLink(AdminFile *source, AdminFile *dest);
	//void RemoveLink(AdminFile *source, AdminFile *dest);
	//AdminFile *GetAdminFile(int kind,const string &name);
	//string FD2Str(int k);
	//string FD2Dir(int k);
	void FraesDir(const string &root_dir, const string &dir, const string &extension);
	void MetaFraesDir(int kind);
	void LoadGameIni(const string &dir, GameIniData *g);
	void SaveGameIni(const string &dir, GameIniData *g);
	void TestRootDirectory();
	void SaveDatabase();
	void ResetDatabase();
	void LoadDatabase();
	void UpdateDatabase();


	AdminFileList file_list;

	Array<string> cft;

	GameIniData GameIni, GameIniExport, GameIniDialog;

	static CFile *admin_file;
};

#endif /* DATAADMINISTRATION_H_ */
