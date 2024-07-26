#pragma once

#include <iostream>
#include <vector>
#include <utility>

#include <asio.hpp>

using tcpip = asio::ip::tcp;
using std::cout;
using std::endl;

class a_socket_receiver{

	public:
	a_socket_receiver(asio::io_context& ioContext, tcpip::endpoint endpoint) : sockAccept(ioContext, endpoint) {
		async_receive();
	}

	int num_new_sockets();

	tcpip::socket pop_socket_back();

	private:
	std::mutex mutex_newSockets;
	std::vector<tcpip::socket> newSockets;

	tcpip::acceptor sockAccept;

	void async_receive();

};
