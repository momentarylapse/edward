//
// Created by Michael Ankele on 2025-04-10.
//

#pragma once

#include <lib/xhui/Dialog.h>


namespace codeedit {

class EditorWindow;

class DocumentSwitcher : public xhui::Dialog {
public:
	explicit DocumentSwitcher(EditorWindow* editor_window);
	~DocumentSwitcher() override;

	void next();
	void previous();

	EditorWindow* editor_window;
};

}

