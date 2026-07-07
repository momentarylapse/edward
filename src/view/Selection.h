//
// Created by michi on 6/1/26.
//

#ifndef EDWARD_SELECTION_H
#define EDWARD_SELECTION_H

#include <lib/base/map.h>

enum class MultiViewType;
using Selection = base::map<MultiViewType, base::set<int>>;

#endif //EDWARD_SELECTION_H
