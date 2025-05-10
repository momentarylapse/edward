/*
 * DataProject.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#pragma once


#include "../../data/Data.h"

class AdminFile;
class AdminFileList;
class GameIniData;

class AdminGameExportException {
public:
	explicit AdminGameExportException(const string &msg) {
		message = msg;
	}
	string message;
};



class DataProject : public Data {
public:
	explicit DataProject(Session *s);
	~DataProject() override;

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
