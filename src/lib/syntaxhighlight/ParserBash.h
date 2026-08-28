//
// Created by michi on 8/29/26.
//

#ifndef EDWARD_PARSERBASH_H
#define EDWARD_PARSERBASH_H

#include "BaseParser.h"

namespace syntaxhighlight {

class ParserBash : public Parser {
public:
	ParserBash();

	Array<Markup> create_markup(const string &text, int offset) override;
};

}


#endif //EDWARD_PARSERBASH_H
