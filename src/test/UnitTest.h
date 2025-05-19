/*
 * UnitTest.h
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef SRC_TEST_UNITTEST_H_
#define SRC_TEST_UNITTEST_H_

#ifndef NDEBUG

#include <lib/base/base.h>
#include <functional>

class Path;
struct vec3;

namespace unittest {

struct TestProtocoll {
	int num_tests_run = 0;
	int num_failed = 0;
};

class UnitTest {
public:
	explicit UnitTest(const string &name);
	virtual ~UnitTest();

	string name;

	class Test {
	public:
		string name;
		std::function<void()> f;
	};
	virtual Array<Test> tests() = 0;

	void run(const string &filter, TestProtocoll &protocoll);
	bool filter_match_group(const string &filter);
	bool filter_match(const string &filter, const string &test_name);

	//void assert(bool )
};


class Failure : public Exception {
public:
	Failure(const string &s) : Exception(s) {}
};
/*template<class T>
static void assert_equal(const T &a, const T&b, const string &text)
{
	if (a != b)
		throw Failure(text);
}*/

void assert_equal(float a, float b, float epsilon = 0.001f);
void assert_equal(const Array<int> &a, const Array<int> &b);
void assert_equal(const Array<float> &a, const Array<float> &b, float epsilon = 0.001f);
void assert_equal(const vec3& a, const vec3& b, float epsilon = 0.001f);

Array<UnitTest*> all();
void run_all(const string &filter);
void print_all_names();

void sleep(float t);

extern Array<string> event_protocoll;
void event(const string &e);
void assert_protocoll(const Array<string> &p);

}

#endif

#endif /* SRC_TEST_UNITTEST_H_ */
