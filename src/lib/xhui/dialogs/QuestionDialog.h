//
// Created by Michael Ankele on 2025-01-26.
//

#ifndef QUESTIONDIALOG_H
#define QUESTIONDIALOG_H

#include "../Dialog.h"
#include "../../base/future.h"

namespace xhui {

enum class Answer {
	Yes,
	No
};

class QuestionDialog : public Dialog {
public:
	QuestionDialog(Panel* parent, const string& title, const string& question, bool allow_cancel = true);

	base::promise<Answer> promise;

	static base::future<Answer> ask(Panel* parent, const string& title, const string& question, bool allow_cancel = true);
};

} // xhui

#endif //QUESTIONDIALOG_H
