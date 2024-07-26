#include <cstdint>
#include <iostream>
#include <vector>
#include <csignal>
#include <thread>
#include <fstream>
#include <string>
#include <map>
#include <ctime>

#include "asio/ip/tcp.hpp"

#include "mdns_cpp/mdns.hpp"

#include "a_socket_find.h"
#include "a_socket_read.h"
#include "utils.h"

using std::cout;
using std::endl;
using tcpip = asio::ip::tcp;

constexpr uint16_t PORT = 46239;

constexpr short LOOPS_PER_SEC = 15;
constexpr short MIN_LOOP_TIME_MS = 1000/LOOPS_PER_SEC;

volatile bool sigintFlag = false;

int main() {

	// INIT stuff

	// read device IDs from file
	std::ifstream IDsFileR("IDs");
	std::map<uint32_t, std::string> IDs;

	std::string readStr = "";
	
	while(IDsFileR){

		readStr = "";
		IDsFileR >> readStr;

		uint32_t ID = 0;
		std::string name = "";
		if(readStr == "") continue;
		for(int i = 0; i < readStr.size(); i++){
			if(readStr[i] == ':'){
				ID = stoul(readStr.substr(0, i), nullptr, 0);
				name = readStr.substr(i+1);
			}
		}
		IDs.emplace(ID, name);
	}
	
	IDsFileR.close();
	std::ofstream IDsFileW;
	IDsFileW.open("IDs", std::ios::app);

	srand(time(NULL));
	int mainRet = 0;
	asio::io_context ioContext;
	a_socket_receiver sockManv4(ioContext, tcpip::endpoint(tcpip::v4(), PORT));
	std::vector <a_socket_rw *> sockReads;

	signal(
		SIGINT,
		[](int signum){
			sigintFlag = true;// stopThreads = true;
		}
	);
	std::thread ioConThr(
		[&ioContext](){
		ioContext.run();
		return;
	}
	);

	// start mdns
	mdns_cpp::mDNS mdns;
	mdns.setServicePort(PORT);
	mdns.setServiceHostname("ESP32CoolandtMonitorServer");
	mdns.setServiceName("_esp32coolmon._tcp.local.");
	mdns.startService();

	std::clock_t loopTime = clock();

	// MAIN LOOP
	while(1){

		while(sockManv4.num_new_sockets() > 0){

			cout << "Main: New Socket" << endl;
			a_socket_rw* tmp = new a_socket_rw(std::move(sockManv4.pop_socket_back()));
			sockReads.push_back(tmp);
		}

		int i = 0;
		for(auto it = sockReads.begin(); it != sockReads.end(); ){
			if((*it)->is_closing()){

				cout << "Deleting socket " << i << endl;
				a_socket_rw * tmp = *it;
				sockReads.erase(it);
				delete tmp;
				it = sockReads.begin();
				i = 0;
			}else{
				it++;
			}

			i++;
		}

		//Read from sockets, also check/assign ID
		for(int i = 0; i < sockReads.size(); i++){
			asio::mutable_buffer * buffp = nullptr;
			bool isFirstRead = sockReads[i]->isFirstRead;
			// If there is no buffer to read then this will just give us nullptr on buffp
			size_t buffSize = sockReads[i]->pop_latest_buff(buffp);

			if(buffp == nullptr) continue;

			if(isFirstRead){


				std::string num = "";
				for(int i = 0; i < buffSize; i++){
					
					if(!isdigit(static_cast<char*>(buffp->data())[i])) continue;
					num.push_back( static_cast<char*>( buffp->data() )[i] );
				}

				uint32_t receivedID = 0;
				if(num.size() > 0){
					receivedID = std::stol(num);
				}
				cout << "receivedID: " << receivedID << endl;

				if(receivedID == 0 || IDs.find(receivedID) == IDs.end()){
					uint32_t randNum = 0;

					do{
						randNum = generateRand(1, 0xffffffff);
					}while(IDs.find(randNum) != IDs.end());

					sockReads[i]->async_write(&randNum, sizeof(uint32_t));

					IDsFileW << randNum << ":NULL\n";
					IDsFileW.flush();

					assert(IDs.emplace(randNum, "NULL").second);
					cout << "New ID: " << randNum << ", With name: NULL" << endl;

				}else{

					sockReads[i]->async_write(&receivedID, sizeof(uint32_t)); // echo the Device ID that was received
				}
			}else{
				cout << "Main, Message: " << static_cast<char*>(buffp->data());
				assert(strcmp((char*)buffp->data(), "<420,20.019199,5.232000\n") == 0);
			}
			free_buffer(buffp);
		}

		//check for Ctrl-c aka sigint
		if(sigintFlag){
			mainRet = 0;
			cout << "SIGINT Received, shutting down" << endl;
			break;
		}

		fflush(stdout);

		std::clock_t current = std::clock();
		std::clock_t tdiff = current - loopTime;
		if(tdiff < MIN_LOOP_TIME_MS){
			std::this_thread::sleep_for(std::chrono::milliseconds(MIN_LOOP_TIME_MS - tdiff));
		}
		// cout << "Time spent in loop: " << tdiff << endl;
		loopTime = std::clock();
	}

	//this first it takes a while sometimes
	std::thread mdnsStopTh([&mdns](){
		mdns.stopService();
	});

	//write file
	IDsFileW.close();
	IDsFileW.open("IDs", std::ios::trunc);
	for(auto i = IDs.begin(); i != IDs.end(); i++){

		IDsFileW << i->first << ':' << i->second << '\n';
	}

	// close sockets
	for(int i = sockReads.size()-1; i >= 0; i--){
		delete sockReads.back();
		sockReads.pop_back();
	}

	//deinit
	ioContext.stop();
	cout << "Stopping network service" << endl;
	ioConThr.join();

	cout << "Stopping mdns service" << endl;
	mdnsStopTh.join();
	cout << "Mdns service stopped" << endl;

	return mainRet;
}
