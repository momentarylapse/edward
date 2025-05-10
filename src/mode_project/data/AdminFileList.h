/*
 * AdminFileList.h
 *
 *  Created on: 02.03.2013
 *      Author: michi
 */

#ifndef ADMINFILELIST_H_
#define ADMINFILELIST_H_

#include "../../data/Data.h"

class AdminFile;
class GameIniData;

class AdminFileList : public Array<AdminFile*> {
public:
	void clear();
	void clear_deep();
	void sort();
	void add_recursive(AdminFile *a);
	AdminFile *add_unchecked(int kind, const Path &filename, AdminFile *source = NULL);
	AdminFile *add_unchecked_ae(int kind, const Path &filename, AdminFile *source);
	AdminFile *get(int kind, const Path &name);
	void remove_obsolete();

	AdminFile *add_engine_files();
	void add_from_game_ini(GameIniData &game_ini, AdminFile *f);
	void add_from_game_ini_export(AdminFileList *source, GameIniData &game_ini);
};


#endif /* ADMINFILELIST_H_ */
