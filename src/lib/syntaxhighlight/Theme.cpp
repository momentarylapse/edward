/*
 * Theme.cpp
 *
 *  Created on: 06.06.2013
 *      Author: michi
 */

#include "Theme.h"

namespace syntaxhighlight {
	static Array<Theme*> HighlightSchemas;
	Theme* default_theme;

	Theme::Theme() {
		bg = White;
		is_default = false;
		changed = false;
	}

	Theme *Theme::copy(const string &name) {
		Theme *s = new Theme;
		*s = *this;
		s->name = name;
		s->is_default = false;
		s->changed = true;
		HighlightSchemas.add(s);
		return s;
	}

	void init() {
		Theme *schema = new Theme;
		schema->name = "default";
		schema->is_default = true;
		schema->bg = White;
		schema->context[(int)MarkupType::LINE_COMMENT] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_1] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_2] = Context{color(1, 0.7f, 0.7f, 0.7f), Black, false, false, true};
		schema->context[(int)MarkupType::SPACE] = Context{Black, Black, false, false, false};
		schema->context[(int)MarkupType::WORD] = Context{Black, Black, false, false, false};
		schema->context[(int)MarkupType::TYPE] = Context{color(1, 0.125f, 0, 0.875f), Black, false, true, false};
		schema->context[(int)MarkupType::GLOBAL_VARIABLE] = Context{color(1, 0.625f, 0.625f, 0), Black, false, false, false};
		schema->context[(int)MarkupType::COMPILER_FUNCTION] = Context{color(1, 0.065f, 0, 0.625f), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR_FUNCTION] = Context{color(1, 0.4f, 0.7f, 0.6f), Black, false, false, false};
		schema->context[(int)MarkupType::SPECIAL] = Context{color(1, 0.625f, 0, 0.625f), Black, false, true, false};
		schema->context[(int)MarkupType::MODIFIER] = Context{color(1, 0.25f, 0.5f, 0.625f), Black, false, false, false};
		schema->context[(int)MarkupType::NUMBER] = Context{color(1, 0, 0.5f, 0), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR] = Context{color(1, 0.25f, 0.25f, 0), Black, false, false, false};
		schema->context[(int)MarkupType::STRING] = Context{color(1, 1, 0, 0), Black, false, false, false};
		schema->context[(int)MarkupType::STRING_SUBSTITUTE] = Context{color(1, 0.6f, 0.3f, 0), Black, false, false, true};
		schema->context[(int)MarkupType::MACRO] = Context{color(1, 0, 0.5f, 0.5f), Black, false, false, false};
		HighlightSchemas.add(schema);

		schema = new Theme;
		schema->name = "dark";
		schema->is_default = true;
		schema->bg = Black;
		schema->context[(int)MarkupType::LINE_COMMENT] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_1] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_2] = Context{color(1, 0.3f, 0.7f, 0.7f), Black, false, false, true};
		schema->context[(int)MarkupType::SPACE] = Context{White, Black, false, false, false};
		schema->context[(int)MarkupType::WORD] = Context{White, Black, false, false, false};
		schema->context[(int)MarkupType::TYPE] = Context{color(1, 0.125f, 0, 0.875f), Black, false, true, false};
		schema->context[(int)MarkupType::GLOBAL_VARIABLE] = Context{color(1, 0.625f, 0.625f, 0), Black, false, false, false};
		schema->context[(int)MarkupType::COMPILER_FUNCTION] = Context{color(1, 0.3f, 0.2f, 1), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR_FUNCTION] = Context{color(1, 0.4f, 0.7f, 0.6f), Black, false, false, false};
		schema->context[(int)MarkupType::SPECIAL] = Context{color(1, 0.625f, 0, 0.625f), Black, false, true, false};
		schema->context[(int)MarkupType::NUMBER] = Context{color(1, 0.3f, 1, 0.3f), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR] = Context{color(1, 1, 1, 0.75f), Black, false, false, false};
		schema->context[(int)MarkupType::STRING] = Context{color(1, 1, 0, 0), Black, false, false, false};
		schema->context[(int)MarkupType::STRING_SUBSTITUTE] = Context{color(1, 0.6f, 0.3f, 0), Black, false, false, true};
		schema->context[(int)MarkupType::MACRO] = Context{color(1, 0, 0.5f, 0.5f), Black, false, false, false};
		HighlightSchemas.add(schema);

		schema = new Theme;
		schema->name = "dark2";
		schema->is_default = true;
		schema->bg = Black;
		schema->context[(int)MarkupType::LINE_COMMENT] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_1] = Context{color(1, 0.5f, 0.5f, 0.5f), Black, false, false, true};
		schema->context[(int)MarkupType::COMMENT_LEVEL_2] = Context{color(1, 0.3f, 0.7f, 0.7f), Black, false, false, true};
		schema->context[(int)MarkupType::SPACE] = Context{White, Black, false, false, false};
		schema->context[(int)MarkupType::WORD] = Context{White, Black, false, false, false};
		schema->context[(int)MarkupType::TYPE] = Context{color(1, 0.4f, 0.4f, 1), Black, false, true, false};
		schema->context[(int)MarkupType::GLOBAL_VARIABLE] = Context{color(1, 1, 0.7f, 0.0f), Black, false, false, false};
		schema->context[(int)MarkupType::COMPILER_FUNCTION] = Context{color(1, 0.4f, 0.4f, 1), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR_FUNCTION] = Context{color(1, 0.4f, 0.7f, 0.6f), Black, false, false, false};
		schema->context[(int)MarkupType::SPECIAL] = Context{color(1, 0.625f, 0, 0.625f), Black, false, true, false};
		schema->context[(int)MarkupType::MODIFIER] = Context{color(1, 0.525f, 0.4f, 0.85f), Black, false, false, false};
		schema->context[(int)MarkupType::NUMBER] = Context{color(1, 0.2f, 0.7f, 0.2f), Black, false, false, false};
		schema->context[(int)MarkupType::OPERATOR] = Context{color(1, 1, 1, 0.3f), Black, false, false, false};
		schema->context[(int)MarkupType::STRING] = Context{color(1, 1, 0, 0), Black, false, false, false};
		schema->context[(int)MarkupType::STRING_SUBSTITUTE] = Context{color(1, 0.6f, 0.3f, 0), Black, false, false, true};
		schema->context[(int)MarkupType::MACRO] = Context{color(1, 0, 0.5f, 0.5f), Black, false, false, false};
		default_theme = schema;
		HighlightSchemas.add(schema);

		InitParser();
	}

	Theme* get_theme(const string &name) {
		if (HighlightSchemas.num == 0)
			init();
		for (Theme *s : HighlightSchemas)
			if (s->name == name)
				return s;
		return HighlightSchemas[0];
	}

	Array<Theme*> get_all_themes() {
		if (HighlightSchemas.num == 0)
			init();
		return HighlightSchemas;
	}
}
