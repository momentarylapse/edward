/*
 * NetworkManager.h
 *
 *  Created on: Jan 8, 2021
 *      Author: michi
 */

#ifndef SRC_NET_NETWORKMANAGER_H_
#define SRC_NET_NETWORKMANAGER_H_

#include <lib/base/base.h>
#include <lib/base/callable.h>
#include <lib/base/pointer.h>
#include <ecs/System.h>
#include <functional>

namespace net {
	class Socket;
}
class BinaryBuffer;

class NetworkManager : public ecs::System {
public:
	static void init();

	NetworkManager();

	struct Connection {
		owned<net::Socket> s;
		owned<BinaryBuffer> buffer;
		bool is_host;

		void start_block(const string &id);
		void end_block();
		void send();
		bool read_block();
	};
	Array<Connection*> connections;
	Connection *cur_con;

	//Socket *socket_to_host;

	Connection* connect_to_host(const string &host);
	void on_iterate(float dt) override;
	void iterate_client(Connection *con);
	void handle_block(Connection *con);

	void send_connect(Connection *con);

	using Callback = Callable<void(VirtualBase*)>;

	struct Observer {
		int hash;
		VirtualBase *object;
		const Callback *callback;
	};
	Array<Observer> observers;
	void event(const string &message, VirtualBase *ob, Callback *cb);

	static const kaba::Class* _class;
};

#endif /* SRC_NET_NETWORKMANAGER_H_ */
