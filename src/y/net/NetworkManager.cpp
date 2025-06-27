/*
 * NetworkManager.cpp
 *
 *  Created on: Jan 8, 2021
 *      Author: michi
 */

#include "NetworkManager.h"
#include "../lib/os/msg.h"
#include "../lib/net/net.h"
#include "../lib/kaba/kaba.h"

const int PORT = 2184;

NetworkManager network_manager;

void NetworkManager::init() {
	NetInit();
}

NetworkManager::NetworkManager() {
	cur_con = nullptr;
}

void NetworkManager::send_connect(Connection *c) {
	c->start_block("y.connect");
	c->end_block();
	c->send();
}

NetworkManager::Connection *NetworkManager::connect_to_host(const string &host) {
	auto con = new Connection;
	con->buffer = new BinaryBuffer;
	con->is_host = false;
	try {
		con->s = net::connect(host, PORT);
		send_connect(con);
		con->s->set_blocking(false);
	} catch (Exception &e) {
		delete con;
		msg_error(e.message());
		return nullptr;
	}
	connections.add(con);
	return con;
}

void NetworkManager::iterate() {
	for (auto con: connections)
		if (!con->is_host)
			iterate_client(con);
}

void NetworkManager::iterate_client(Connection *con) {
	if (!con->s->can_read())
		return;
	if (con->read_block()) {
		handle_block(con);
		con->buffer->clear();
	}
}

void NetworkManager::Connection::start_block(const string &id) {
	*buffer << id.hash();
	buffer->start_block();
}

void NetworkManager::Connection::end_block() {
	buffer->end_block();
}

void NetworkManager::Connection::send() {
	//msg_write("sending " + buffer->data.hex());
	s->write(buffer->data);
	buffer->clear();
}

bool NetworkManager::Connection::read_block() {
	//msg_write("NM rb");
	const int FRAME = 8;

	// 8b frame
	if (buffer->data.num < FRAME)
		buffer->data += s->read(FRAME - buffer->data.num);
	if (buffer->data.num < FRAME)
		return false;
	//msg_write("NM rb2");
	buffer->set_pos(0);
	int hash, size;
	*buffer >> hash;
	*buffer >> size;
	buffer->set_pos(0);
	if (buffer->data.num < FRAME + size)
		buffer->data += s->read(FRAME + size - buffer->data.num);
	if (buffer->data.num < FRAME + size)
		return false;
	//msg_write("NM rb3");
	return true;

}

void NetworkManager::handle_block(Connection *con) {
	//msg_write("NM handle block");
	cur_con = con;

	int hash, size;
	*con->buffer >> hash;
	*con->buffer >> size;

	int pos0 = con->buffer->get_pos();

	for (auto &o: observers)
		if (o.hash == hash) {
			//msg_error("JAAAAAA");
			(*o.callback)(o.object);
			break;
		}

	con->buffer->set_pos(pos0 + size);
}

void NetworkManager::event(const string &message, VirtualBase *ob, Callback *cb) {
	//msg_write("NM event " + p2s((void*)cb));
	observers.add({message.hash(), ob, cb});
}

