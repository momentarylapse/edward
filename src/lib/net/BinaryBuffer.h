/*
 * BinaryBuffer.h
 *
 *  Created on: Jan 8, 2021
 *      Author: michi
 */

#ifndef SRC_LIB_NET_BINARYBUFFER_H_
#define SRC_LIB_NET_BINARYBUFFER_H_


#include "../base/base.h"

class vector;


class BinaryBuffer {
public:
	BinaryBuffer();
	~BinaryBuffer() {}

	void __init__();
	void __delete__();

	string data;
	int pos;
	void set_pos(int pos);
	int get_pos();
	void clear();


	// argh...
	int block_pos;
	void start_block();
	void end_block();

	void read(void *p, int size);

	// read
	void operator>>(int &i);
	void operator>>(float &f);
	void operator>>(bool &b);
	void operator>>(char &c);
	void operator>>(string &s);
	void operator>>(vector &v);

	// write
	void operator<<(int i);
	void operator<<(float f);
	void operator<<(bool b);
	void operator<<(char c);
	void operator<<(const string &s);
	void operator<<(const vector &v);
};



#endif /* SRC_LIB_NET_BINARYBUFFER_H_ */
