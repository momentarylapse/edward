//
// Created by michi on 9/21/25.
//

#pragma once

#include <view/Mode.h>
#include <data/Data.h>
#include <lib/base/pointer.h>

class Path;
class CodingPanel;


class CodeData : public Data {
public:
	CodeData(DocumentSession* doc);
	void reset() override {}
};


class ModeCoding : public Mode {
public:
	explicit ModeCoding(DocumentSession* doc);
	~ModeCoding() override;

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_command(const string& id) override;

	void on_set_menu() override;
	void update_menu();

	void load(const Path& filename);

	CodingPanel* coding_panel;
	owned<CodeData> data;
};

