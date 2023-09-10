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
	DataAdministration(EdwardWindow *ed);
	virtual ~DataAdministration();

	void reset();
	bool load(const Path &_filename, bool deep = true);
	bool save(const Path &_filename);

	void FraesDir(const Path &root_dir, const Path &dir, const string &extension);
	void MetaFraesDir(int kind);
	void TestRootDirectory();
	void SaveDatabase();
	void LoadDatabase();
	void UpdateDatabase();

	void ExportGame(const Path &dir, GameIniData &game_ini);

	AdminFileList *file_list;

	Array<Path> cft;

	GameIniData *GameIni;
};

#endif /* DATAADMINISTRATION_H_ */
