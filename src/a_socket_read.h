#pragma once

#include <iostream>
#include <vector>
#include <utility>

#include <asio.hpp>

using tcpip = asio::ip::tcp;
using std::cout;
using std::endl;

constexpr int maxReadSize = 256;

// NOTE: NEVER, and i mean never, delete an a_socket_rw object without is_closing() returning true
// The ioContext which this is a class of should process requests through the lifetime of this object
class a_socket_rw{

	public:
	a_socket_rw(tcpip::socket sock) : our_socket(std::move(sock)) {
		async_read();
	}

	int num_new_buffers();

	bool is_closing();

	// YOU OWN THE BUFFER NOW!!!
	// You must free it with free_buffer()
	// this function will give you only the latest buffer, then delete the rest
	size_t pop_latest_buff(asio::mutable_buffer * & buff);

	// self explanitory, writes to socket asynchronously and handles the result
	void async_write(void const * const buff, size_t size_bytes);

	~a_socket_rw();

	bool isFirstRead = true;
	uint32_t device_ID = 0;

	private:
	std::mutex mutex_this;
	std::vector<asio::mutable_buffer *> newBuffers;
	std::vector<size_t> buffSizes;
	tcpip::socket our_socket;
	time_t TimeOfLastMsg = time(NULL);//time since laste messageg
	int numOutStandingOps = 0;
	char messageBuff[maxReadSize*2] = {0};

	void cancel();

	void flush_helper();

	void async_read();
};

void free_buffer(asio::mutable_buffer * buff);

void free_buffer(asio::const_buffer * buff);
