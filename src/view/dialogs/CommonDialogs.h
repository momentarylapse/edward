//
// Created by Michael Ankele on 2025-04-26.
//

#ifndef COMMONDIALOGS_H
#define COMMONDIALOGS_H

#include <lib/xhui/Dialog.h>
#include <lib/base/tuple.h>
#include <lib/base/future.h>
#include <lib/base/optional.h>

class IntPairDialog : public xhui::Dialog {
public:
	IntPairDialog(xhui::Panel* parent, const string& question, int a, int b);
	base::optional<base::tuple<int,int>> answer;
	base::promise<base::tuple<int,int>> promise;

	static base::future<base::tuple<int,int>> ask(xhui::Panel* parent, const string& question, int a, int b);
};

class FloatDialog : public xhui::Dialog {
public:
	FloatDialog(xhui::Panel* parent, const string& question, float f);
	base::optional<float> answer;
	base::promise<float> promise;

	static base::future<float> ask(xhui::Panel* parent, const string& question, float f);
};

class TextDialog : public xhui::Dialog {
public:
	TextDialog(xhui::Panel* parent, const string& question, const string& s);
	base::optional<string> answer;
	base::promise<string> promise;

	static base::future<string> ask(xhui::Panel* parent, const string& question, const string& s);
};

#endif //COMMONDIALOGS_H
