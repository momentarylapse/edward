//
// Created by michi on 9/21/25.
//

#pragma once

#include <view/Mode.h>
#include <lib/base/pointer.h>

class Path;
class CodingPanel;
class CodeData;


class ModeCoding : public Mode {
public:
	explicit ModeCoding(DocumentSession* doc);
	~ModeCoding() override;

	void on_enter_rec() override;
	void on_leave_rec() override;
	void on_command(const string& id) override;

	void update_menu();

	void load(const Path& filename);

	CodingPanel* coding_panel;
	owned<CodeData> data;
};

