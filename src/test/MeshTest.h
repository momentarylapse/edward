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

	static void test_diff_basic_vertices();
	static void test_diff_invertible();
	static void test_diff_iterated();
};

}

#endif //MESHTEST_H

#endif
