/*
 * DataAdministration.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef DATAADMINISTRATION_H_
#define DATAADMINISTRATION_H_

#include "../Data.h"

class AdminFile;
class AdminFileList;
class GameIniData;

class AdminGameExportException
{
public:
	AdminGameExportException(const string &msg){	message = msg;	}
	string message;
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

	AdminFileList *file_list;

	Array<string> cft;

	GameIniData *GameIni;

	static CFile *admin_file;
};

#endif /* DATAADMINISTRATION_H_ */
