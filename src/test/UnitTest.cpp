/*
 * UnitTest.cpp
 *
 *  Created on: 22.07.2018
 *      Author: michi
 */

#ifndef NDEBUG

#include "UnitTest.h"
#include "../lib/os/msg.h"
#include "../lib/os/time.h"
#include "../lib/os/terminal.h"
//#include "../lib/hui/hui.h"
#include <lib/mesh/PolygonMesh.h>
#include <cmath>
#include <cstdio>

#include "MeshTest.h"


namespace unittest {

Array<string> event_protocoll;

UnitTest::UnitTest(const string &_name) {
	name = _name;
}

UnitTest::~UnitTest() = default;

void UnitTest::run(const string &filter, TestProtocoll &protocoll) {
	msg_write(format("%s==  %s  ==%s", os::terminal::BOLD, name, os::terminal::END));
	for (auto &t: tests()) {
		if (!filter_match(filter, t.name))
			continue;
		event_protocoll.clear();
		printf("%s", t.name.c_str());
		try {
			t.f();
			//msg_write("  ok");
			msg_write(os::terminal::GREEN + "   ok " + os::terminal::END);
		} catch(Failure &e) {
			puts(os::terminal::RED.c_str());
			msg_error(e.message());
			protocoll.num_failed ++;
			puts(os::terminal::END.c_str());
		}
		protocoll.num_tests_run ++;
	}
}

bool UnitTest::filter_match(const string &filter, const string &test_name) {
	if (!filter_match_group(filter))
		return false;
	auto xx = filter.explode("/");
	if (xx.num < 2)
		return true;
	return test_name.match(xx[1]);
}

bool UnitTest::filter_match_group(const string &filter) {
	auto xx = filter.explode("/");
	if (xx[0] == "all" or xx[0] == "*")
		return true;
	if (name.match(xx[0]))
		return true;
	return false;
}

void sleep(float t) {
	os::Timer timer;
	while (timer.peek() < t) {
		//hui::Application::do_single_main_loop();
		os::sleep(0.001f);
	}
}

void assert_equal(float a, float b, float epsilon) {
	if (fabs(a - b) > epsilon)
		throw Failure("a!=b\na: " + f2s(a, 6) + "\nb: " + f2s(b, 6));
}

void assert_equal(const Array<int> &a, const Array<int> &b) {
	if (a.num != b.num)
		throw Failure(format("a.num (%d) != b.num (%d)", a.num, b.num));
	for (int i=0; i<a.num; i++)
		if (a[i] != b[i])
			throw Failure("a!=b\na: " + str(a) + "\nb: " + str(b));
}

void assert_equal(const Array<float> &a, const Array<float> &b, float epsilon) {
	if (a.num != b.num)
		throw Failure(format("a.num (%d) != b.num (%d)", a.num, b.num));
	for (int i=0; i<a.num; i++)
		if (fabs(a[i] - b[i]) > epsilon)
			throw Failure("a!=b\na: " + str(a) + "\nb: " + str(b));
}

void assert_equal(const vec3& a, const vec3& b, float epsilon) {
	if ((a - b).length() > epsilon)
		throw Failure("a!=b\na: " + str(a) + "\nb: " + str(b));
}

void event(const string &e) {
	event_protocoll.add(e);
}

void assert_protocoll(const Array<string> &p) {
	if (p != event_protocoll)
		throw Failure("Events: " + str(p) + "  expected: " + str(event_protocoll));
}

Array<UnitTest*> all() {
	Array<UnitTest*> tests;
	tests.add(new MeshTest);
	return tests;
}

void run_all(const string &filter) {
	auto tests = all();

	TestProtocoll protocoll;

	for (auto *t: tests)
		if (t->filter_match_group(filter))
			t->run(filter, protocoll);

	for (auto *t: tests)
		delete t;


	msg_write("\n");
	if (protocoll.num_failed > 0) {

		puts(os::terminal::RED.c_str());
		msg_error(format("%d out of %d tests failed", protocoll.num_failed, protocoll.num_tests_run));
		puts(os::terminal::END.c_str());
	} else {
		puts(os::terminal::GREEN.c_str());
		msg_write("-----------------------------");
		msg_write(format("all %d tests succeeded", protocoll.num_tests_run));
		msg_write("-----------------------------");
		puts(os::terminal::END.c_str());
	}
}

void print_all_names() {
	auto tests = all();

	msg_write("available tests:");
	for (auto *t: tests)
		msg_write("  " + t->name);
	msg_write("  all");

	for (auto *t: tests)
		delete t;
}

}

#endif
