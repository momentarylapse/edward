//
// Created by Michael Ankele on 2025-01-26.
//

#ifndef FILESELECTIONDIALOG_H
#define FILESELECTIONDIALOG_H

#include <lib/base/optional.h>

#include "../Dialog.h"
#include "../../base/future.h"
#include "../../os/path.h"

namespace xhui {

class FileSelector;

class FileSelectionDialog : public Dialog {
public:
	FileSelectionDialog(Panel* parent, const string& title, const Path &dir, const Array<string> &params);

	FileSelector* selector;
	base::optional<Path> answer;
	base::promise<Path> promise;

	bool saving;
	Path selected_path() const;

	static base::future<Path> ask(Panel* parent, const string& title, const Path &dir, const Array<string> &params);
};

} // xhui

#endif //FILESELECTIONDIALOG_H
