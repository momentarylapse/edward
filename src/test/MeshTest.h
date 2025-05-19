//
// Created by michi on 19.05.25.
//

#ifndef NDEBUG

#ifndef MESHTEST_H
#define MESHTEST_H

#include "UnitTest.h"

namespace unittest {

class MeshTest : public UnitTest {
public:
	MeshTest();

	Array<Test> tests() override;

	static void test_diff_invertible();
};

}

#endif //MESHTEST_H

#endif
