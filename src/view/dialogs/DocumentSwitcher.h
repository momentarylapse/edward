//
// Created by Michael Ankele on 2025-04-10.
//

#pragma once

#include <lib/xhui/Dialog.h>

class EdwardWindow;

class DocumentSwitcher : public xhui::Dialog {
public:
	explicit DocumentSwitcher(EdwardWindow* window);
	~DocumentSwitcher() override;

	void next();
	void previous();

	EdwardWindow* editor_window;
};

