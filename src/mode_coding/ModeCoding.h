//
// Created by michi on 9/21/25.
//

#pragma once

#include <view/Mode.h>
#include <data/Data.h>
#include <lib/base/pointer.h>

class Path;
//class CodingPanel;

namespace codeedit {
	class CodeEditor;
}


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
	void on_connect_events_rec() override;
	void on_command(const string& id) override;

	void on_set_menu() override;
	void update_menu();

	bool is_save_state() const override;
	bool is_undoable() const override;
	bool is_redoable() const override;

	void load(const Path& filename);

	codeedit::CodeEditor* editor;
	owned<CodeData> data;
};

