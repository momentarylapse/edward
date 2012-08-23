/*
 * DataAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef DATAADMINISTRATION_H_
#define DATAADMINISTRATION_H_

#include "../Data.h"



struct AdminFile{
	string Name;
	int Kind;
	bool Missing;
	bool Checked;
	Array<AdminFile*> Source;
	Array<AdminFile*> Dest;
	int Time;
};


typedef Array<AdminFile*> AdminFileList;


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

	void AddLink(AdminFile *source, AdminFile *dest);
	void RemoveLink(AdminFile *source, AdminFile *dest);
	AdminFile *GetAdminFile(int kind,const string &name);
	string FD2Str(int k);
	string FD2Dir(int k);
	string StringAfterString;
	bool StringBegin(const string &buf, int start, const string &test);
	void add_possible_link(Array<s_admin_link> &l, int type, const string &filename);
	void CheckFile(AdminFile *a);
	AdminFile *AddFileUnchecked(int kind, const string &filename, AdminFile *source = NULL);
	AdminFile *AddFileUnchecked_ae(int kind, const string &filename, AdminFile *source);
	void FraesDir(const string &root_dir, const string &dir, const string &extension);
	void MetaFraesDir(int kind);
	void LoadGameIni(const string &dir, GameIniData *g);
	void SaveGameIni(const string &dir, GameIniData *g);
	void TestRootDirectory();
	void SaveDatabase();
	void ResetDatabase();
	void LoadDatabase();
	void UpdateDatabase();
	void FindRecursive(AdminFileList &a, AdminFile *to_add, bool source, int levels);
	void SortList(AdminFileList &a);


	AdminFileList file_list_all; // all files (the ones already found...)

	Array<string> cft;

	GameIniData GameIni, GameIniExport, GameIniDialog;

	CFile *admin_file;
};

#endif /* DATAADMINISTRATION_H_ */
