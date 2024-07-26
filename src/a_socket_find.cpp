#include <iostream>

#include <asio.hpp>

#include "a_socket_find.h"

using tcpip = asio::ip::tcp;
using std::cout;
using std::endl;

tcpip::socket a_socket_receiver::pop_socket_back() {

	mutex_newSockets.lock();

	tcpip::socket tmpsock = std::move(newSockets.back());
	newSockets.pop_back();

	mutex_newSockets.unlock();
	return tmpsock;
}

int a_socket_receiver::num_new_sockets() {

	mutex_newSockets.lock();

	int num = newSockets.size();

	mutex_newSockets.unlock();

	return num;
}

void a_socket_receiver::async_receive() {

	sockAccept.async_accept(
		[this](asio::error_code e, tcpip::socket new_socket)
		{

			if(e){
				cout << "async_accept error " << e.value() << ": " << e.message() << endl;
				return;
			}else{
				cout << "Successfully accepted socket" << endl;

				mutex_newSockets.lock();

				newSockets.push_back(std::move(new_socket));

				mutex_newSockets.unlock();
			}
			this->async_receive();
			return;
		}
	);
}
