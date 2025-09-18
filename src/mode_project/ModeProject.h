/*
 * ModeProject.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#pragma once


#include "../view/Mode.h"

class DataProject;
class ProjectDialog;
class Path;

class ModeProject: public Mode {
public:
	explicit ModeProject(DocumentSession *s);
	~ModeProject() override;

	void _new();
	bool open();

	void on_enter() override;
	void on_leave() override;

	void on_command(const string &id) override;

	void basic_settings();
	void export_game();

	void create_project(const Path &dir, const string &first_world);
	void upgrade_project(const Path &dir);

	//AdministrationDialog *dialog;

	DataProject* data;
};
