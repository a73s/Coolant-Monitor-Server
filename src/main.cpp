#include <cstdint>
#include <iostream>
#include <vector>
#include <csignal>
#include <thread>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <future>

#include "asio/ip/tcp.hpp"

#include "mdns_cpp/mdns.hpp"
#include "mdns_cpp/logger.hpp"

#include "a_socket_find.h"
#include "a_socket_read.h"
#include "ui.h"
#include "utils.h"

// using std::cout;
// using std::endl;
using tcpip = asio::ip::tcp;

constexpr uint16_t PORT = 46239;

constexpr short LOOPS_PER_SEC = 15;
constexpr short MIN_LOOP_TIME_MS = 1000/LOOPS_PER_SEC;

volatile bool sigintFlag = false;

int main() {

	// INIT stuff

	// read device IDs from file
	cursesUi ui;
	std::ifstream IDsFileR("IDs");
	std::map<uint32_t, std::string> IDs;
	std::vector<std::future<std::string>> nameFutures;

	std::string readStr = "";
	
	while(IDsFileR){

		readStr = "";
		IDsFileR >> readStr;

		uint32_t ID = 0;
		std::string name = "";
		if(readStr == "") continue;
		for(size_t i = 0; i < readStr.size(); i++){
			if(readStr[i] == ':'){
				ID = stoul(readStr.substr(0, i), nullptr, 0);
				name = readStr.substr(i+1);
			}
		}
		if(name == "NULL"){
			nameFutures.push_back(ui.getDeviceName());
		}
		IDs.emplace(ID, name);
	}
	
	IDsFileR.close();

	srand(time(NULL));
	int mainRet = 0;
	asio::io_context ioContext;
	a_socket_receiver sockManv4(ioContext, tcpip::endpoint(tcpip::v4(), PORT));
	std::vector <a_socket_rw *> sockReads;

	signal(
		SIGINT,
		[](int signum){
			sigintFlag = true;// stopThreads = true;
			std::cerr << signum;
		}
	);
	std::thread ioConThr(
		[&ioContext](){
			ioContext.run();
			return;
		}
	);

	// start mdns
	mdns_cpp::Logger::setLoggerSink(
		[&ui](std::string str){
			ui.printo(str);
		}
	);

	mdns_cpp::mDNS mdns;
	mdns.setServicePort(PORT);
	mdns.setServiceHostname("ESP32CoolandtMonitorServer");
	mdns.setServiceName("_esp32coolmon._tcp.local.");
	mdns.startService();

	std::clock_t loopTime = clock();

	// MAIN LOOP
	while(1){

		while(sockManv4.num_new_sockets() > 0){

			ui.printo("Main: New Socket\n");
			a_socket_rw* tmp = new a_socket_rw(sockManv4.pop_socket_back());
			sockReads.push_back(tmp);
		}

		int i = 0;
		for(auto it = sockReads.begin(); it != sockReads.end(); ){
			if((*it)->is_closing()){

				ui.printo("Deleting socket " + std::to_string(i) + "\n");
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
		for(size_t i = 0; i < sockReads.size(); i++){
			asio::mutable_buffer * buffp = nullptr;
			bool isFirstRead = sockReads[i]->isFirstRead;
			// If there is no buffer to read then this will just give us nullptr on buffp
			size_t buffSize = sockReads[i]->pop_latest_buff(buffp);

			if(buffp == nullptr) continue;

			if(isFirstRead){

				std::string num = "";
				for(size_t i = 0; i < buffSize; i++){
					
					if(!isdigit(static_cast<char*>(buffp->data())[i])) continue;
					num.push_back( static_cast<char*>( buffp->data() )[i] );
				}

				uint32_t receivedID = 0;
				if(num.size() > 0){
					receivedID = std::stol(num);
				}
				ui.printo("receivedID: " + std::to_string(receivedID) + "\n");

				if(receivedID == 0 || IDs.find(receivedID) == IDs.end()){
					uint32_t randNum = 0;

					do{
						randNum = generateRand(1, 0xffffffff);
					}while(IDs.find(randNum) != IDs.end());

					sockReads[i]->async_write(&randNum, sizeof(uint32_t));

					std::ofstream IDsFileWApp;
					IDsFileWApp.open("IDs", std::ios::app);
					IDsFileWApp << randNum << ":NULL\n";
					IDsFileWApp.close();

					assert(IDs.emplace(randNum, "NULL").second);
					ui.printo("New ID: "+std::to_string(randNum)+", With name: NULL\n");

				}else{

					sockReads[i]->async_write(&receivedID, sizeof(uint32_t)); // echo the Device ID that was received
				}
			}else{
				ui.printo("Main, Message: " + std::string(static_cast<char*>(buffp->data())));
				assert(strcmp((char*)buffp->data(), "<420,20.019199,5.232000\n") == 0);
			}
			free_buffer(buffp);
		}

		// resolve ready name futures
		bool updatedAName = false;
		for(auto futures_it = nameFutures.begin(); futures_it != nameFutures.end(); futures_it++){
			if(nameFutures[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready){
				for(auto ids_it = IDs.begin(); ids_it != IDs.end(); ids_it++){

					if(ids_it->second == "NULL"){

						ids_it->second = futures_it->get();
						updatedAName = true;

						auto future_del = futures_it;
						futures_it--;
						nameFutures.erase(future_del);

						goto nextfuture;
					}
				}
			}
			nextfuture:;
		}

		if(updatedAName){

			std::ofstream IDsFileW;
			IDsFileW.open("IDs", std::ios::trunc);
			mapToFile(IDsFileW, IDs);
			IDsFileW.close();
		}

		// check for new command
		std::string commandString = ui.getCommand();
		if(commandString != ""){
			ui.printc("############# Command Entered: " + commandString);
		}

		//check for Ctrl-c aka sigint
		if(sigintFlag){
			mainRet = 0;
			ui.printo("SIGINT Received, shutting down");
			break;
		}

		ui.update();

		std::clock_t current = std::clock();
		std::clock_t tdiff = current - loopTime;
		if(tdiff < MIN_LOOP_TIME_MS){
			std::this_thread::sleep_for(std::chrono::milliseconds(MIN_LOOP_TIME_MS - tdiff));
		}
		loopTime = std::clock();
	}

	//this first it takes a while sometimes
	std::thread mdnsStopTh([&mdns](){
		mdns.stopService();
	});

	//write file
	std::ofstream IDsFileW;
	IDsFileW.open("IDs", std::ios::trunc);
	mapToFile(IDsFileW, IDs);
	IDsFileW.close();

	// close sockets
	for(int i = sockReads.size()-1; i >= 0; i--){
		delete sockReads.back();
		sockReads.pop_back();
	}

	//deinit
	ioContext.stop();
	ui.printoImmediate("Stopping network service\n");
	ioConThr.join();

	ui.printoImmediate("Stopping mdns servicen\n");
	mdnsStopTh.join();
	ui.printoImmediate("Mdns service stopped\n");

	return mainRet;
}
