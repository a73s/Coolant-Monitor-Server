#include <iostream>
#include <ctime>

#include <asio.hpp>

#include "a_socket_read.h"

#define TIMEOUT_SECONDS 60

using tcpip = asio::ip::tcp;
using std::cout; using std::endl;

// The job of this function is, given a new buffer and the leftovers from the previous buffers (stored in messageBuff)
// to set up buff to contain only the most recent newline-seperated "message"
// remnants of an incomplete message should be stored in messageBuff,
// the number of bytes in buff is returned through ret_messageSize
//
// There is an assumption that the message is text so anything after the first null byte will be cut off
inline bool manageMessageBuffers(char * const messageBuff, uint32_t messagebuffmax, asio::mutable_buffer * const buff, size_t readsize, uint16_t & ret_messageSize){

	//sorry but this is a doozy

	ret_messageSize = readsize;
	uint16_t mbufflen = strlen(messageBuff);

	// guard messageBuff from overflow
	if(mbufflen + readsize > messagebuffmax){

		messageBuff[0] = '\0';
		static_cast<char*>( buff->data() )[0] = '\0';
		ret_messageSize = 0;
		return false;
	}

	// concatinate buff->data onto messagebuff
	for(uint16_t i = 0; i < readsize; i++){
		messageBuff[i+mbufflen] = static_cast<char*>(buff->data())[i];
	}
	messageBuff[readsize+mbufflen] = '\0';

	mbufflen += readsize;

	// assert(mbufflen == strlen(messageBuff));
	mbufflen = strlen(messageBuff);

	// identify the last full message by finding final newline
	int16_t lastNewlineCharIndex = -1;
	for(uint16_t i = 0; i < mbufflen; i++){
		if( messageBuff[i] == '\n'){
			lastNewlineCharIndex = i;
		}
	}

	if(lastNewlineCharIndex != -1){
		// find starting index of the message
		uint16_t latestMessageStartIndex = 0;
		for(int16_t i = lastNewlineCharIndex-1; i >= 0; i--){
			if(messageBuff[i] == '\n'){
				latestMessageStartIndex = i+1;
				break;
			}
		}

		//copy latest message back into buffer;
		for(uint16_t i = latestMessageStartIndex; i <= lastNewlineCharIndex; i++){
			static_cast<char*>(buff->data())[i - latestMessageStartIndex] = messageBuff[i];
		}

		//add null terminator on buffer
		for(uint16_t i = 1; i < messagebuffmax; i++){
			if(static_cast<char*>(buff->data())[i-1] == '\n'){
				static_cast<char*>(buff->data())[i] = '\0';
				break;
			}
		}

		//shift remaining message remnantes
		for(uint16_t i = lastNewlineCharIndex + 1; messageBuff[i-1] != '\0'; i++){
			messageBuff[i-(lastNewlineCharIndex + 1)] = messageBuff[i];
		}

		ret_messageSize = lastNewlineCharIndex - latestMessageStartIndex + 1;

		assert(ret_messageSize == strlen(static_cast<char*>(buff->data())));

		return true;
	}
	return false;
}

// YOU OWN THE BUFFER NOW!!!
// You must free it with free_buffer()
size_t a_socket_rw::pop_latest_buff(asio::mutable_buffer * & buff) {

	mutex_this.lock();

	size_t size = 0;
	if(newBuffers.size() > 0 && (numOutStandingOps > 0)){

		buff = newBuffers.back();
		size = buffSizes.back();

		newBuffers.pop_back();
		buffSizes.pop_back();

		flush_helper();

		isFirstRead = false;
	}

	if(difftime(time(NULL), TimeOfLastMsg) > TIMEOUT_SECONDS){

		cout << "Socket timed out" << endl;
		our_socket.close();
	}

	mutex_this.unlock();

	return size;
}

bool a_socket_rw::is_closing() {

	mutex_this.lock();

	bool tmp = (numOutStandingOps == 0);

	mutex_this.unlock();
	
	return tmp;
}

int a_socket_rw::num_new_buffers() {

	mutex_this.lock();

	int num = newBuffers.size();

	mutex_this.unlock();

	return num;
}

void a_socket_rw::async_write(void const * const buff, size_t size_bytes) {

	void * const newBuffPtr = new uint8_t[size_bytes];
	for(int i = 0; i < size_bytes; i++){
		static_cast<uint8_t * const>(newBuffPtr)[i] = static_cast<uint8_t const * const>(buff)[i];
	}
	
	asio::const_buffer * buffptr = new asio::const_buffer(newBuffPtr, size_bytes);
	mutex_this.lock();
	numOutStandingOps++;

	our_socket.async_send(
		*buffptr,
		[this, buffptr](asio::error_code e, size_t sendsize){

			this->mutex_this.lock();
			this->numOutStandingOps--;
			switch(e.value()){

				case 0: {//success

					this->TimeOfLastMsg = time(NULL);

					break;
				}
				case 2:{

					cout << "Socket closed by client" << endl;

					this->our_socket.close();
					break;
				}
				case 125:{

					cout << "Async socket send canceled" << endl;

					this->our_socket.close();

					break;
				}
				default:{

					cout << "Unhandled async_send error " << e.value() << ": " << e.message() << endl;

					this->our_socket.close();

					break;
				}
			}
			free_buffer(buffptr);
			this->mutex_this.unlock();
			return;
		}
	);
	mutex_this.unlock();
}

void a_socket_rw::async_read() {

	void * buffdata = new uint8_t[maxReadSize];
	asio::mutable_buffer * buff = new asio::mutable_buffer(buffdata, maxReadSize);

	// no need to lock the mutex here. in the case of the first run
	// there is no code running in another thread, on subsequent calls
	// the mutex will already have been locked in the callback func
	// this also allows us to guarantee that nomOutstandingOps will
	// not drop to zero between unlocking in the callback and locking
	// in the async_read() function, if it did we may spuriously return
	// true from is_closing()
	numOutStandingOps++;

	our_socket.async_receive(*buff,
		[this, buff](asio::error_code e, size_t readsize)
		{

			this->mutex_this.lock();
			switch(e.value()){

				case 0: {//success

					this->TimeOfLastMsg = time(NULL);
					this->numOutStandingOps--;

					assert(readsize != 0);

					uint16_t messageSize = 0;

					if(manageMessageBuffers(this->messageBuff, sizeof this->messageBuff, buff, readsize, messageSize)){
						newBuffers.push_back(buff);
						buffSizes.push_back(messageSize);
					}

					this->async_read();
					break;
				}
				case 2:{//closed by client

					cout << "Socket closed by client" << endl;
					this->our_socket.close();
					this->numOutStandingOps--;

					break;
				}
				case 125:{//operation aborted

					cout << "Async socket read cancelled" << endl;
					this->our_socket.close();
					this->numOutStandingOps--;

					break;
				}
				default:{//other error

					cout << "Unhandled async_receive error " << e.value() << ": " << e.message() << endl;
					this->our_socket.close();
					this->numOutStandingOps--;
					break;
				}
			}
			if(e){
				free_buffer(buff);
			}
			this->mutex_this.unlock();
			return;
		}
	);
}

void a_socket_rw::flush_helper(){

	while(newBuffers.size() > 0){
		free_buffer(newBuffers.back());
		newBuffers.pop_back();
		buffSizes.pop_back();
	}
}

void a_socket_rw::cancel(){

	mutex_this.lock();
	if(our_socket.is_open()){
		our_socket.cancel();
	}
	mutex_this.unlock();
}

a_socket_rw::~a_socket_rw() {


	cancel();
	while(!is_closing());

	mutex_this.lock();

	flush_helper();
	
	mutex_this.unlock();

}

void free_buffer(asio::mutable_buffer * buff){
	
	delete[] static_cast<uint8_t*>(buff->data());
	delete buff;
}

void free_buffer(asio::const_buffer * buff){
	
	delete[] static_cast<uint8_t const *>(buff->data());
	delete buff;
}
