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
#include <cassert>
#include <curl/curl.h>

#include "asio/ip/tcp.hpp"

#include "mdns_cpp/mdns.hpp"
#include "mdns_cpp/logger.hpp"

#include "a_socket_find.h"
#include "a_socket_read.h"
#include "ui.h"
#include "utils.h"
#include "data_handling.h"

using tcpip = asio::ip::tcp;

constexpr uint16_t MDNS_PORT = 46239;

constexpr short LOOPS_PER_SEC = 15;
constexpr short MIN_LOOP_TIME_MS = 1000/LOOPS_PER_SEC;

volatile bool sigintFlag = false;

int main() {

	// INIT stuff
	CURL* curl = curl_easy_init();

	//Retrieve api key for automated texting
	std::ifstream apiFile("apikey.txt");
	std::string apiKey = "";
	if(apiFile.is_open()){
		apiFile >> apiKey;
		if(std::isspace(apiKey.back())){
			apiKey.pop_back();
		}
	}else{
		std::cout << "Failed to Retrieve api key. Shutting down" << std::endl;
		exit(1);
	}
	apiFile.close();

	//Retrieve the list of phone numbers to be notified
	std::ifstream phonesFileR("phones.txt");
	std::vector<std::string> phones;
	while(phonesFileR){
		std::string phone;
		std::getline(phonesFileR, phone);
		for(size_t i = 0; i < phone.size(); i++){
			if(!isdigit(phone[i])){
				phone.erase(i);
				i--;
			}
		}

		phones.push_back(phone);
	}

	// read device IDs from file
	cursesUi ui;
	std::ifstream IDsFileR("IDs");
	std::map<uint32_t, std::string> IDs;
	std::vector<std::future<std::string>> nameFutures;
	std::vector<dataMonitor> dataMonitors;

	std::string readStr = "";
	
	while(IDsFileR){

		readStr = "";
		std::getline(IDsFileR, readStr);

		uint32_t ID = 0;
		std::string name = "";
		if(readStr == "") continue;
		for(size_t i = 0; i < readStr.size(); i++){
			if(readStr[i] == ':'){
				ID = stoul(readStr.substr(0, i), nullptr, 0);
				name = readStr.substr(i+1);
				break;
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
	a_socket_receiver sockManv4(ioContext, tcpip::endpoint(tcpip::v4(), MDNS_PORT));
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
	mdns.setServicePort(MDNS_PORT);
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
			dataMonitors.push_back(dataMonitor());
		}

		//delete dead sockets
		int i = 0;
		for(auto it = sockReads.begin(); it != sockReads.end(); ){
			if((*it)->is_closing()){

				ui.printo("Deleting socket " + std::to_string(i) + "\n");
				a_socket_rw * tmp = *it;
				sockReads.erase(it);
				delete tmp;
				it = sockReads.begin();
				//delete the data monitor associated with the socket
				dataMonitors.erase(dataMonitors.begin()+i);
				i = 0;
			}else{
				it++;
				i++;
			}
		}

		//Read from sockets, also check/assign ID
		for(size_t i = 0; i < sockReads.size(); i++){
			asio::mutable_buffer * buffp = nullptr;
			bool isFirstRead = sockReads[i]->isFirstRead;
			// If there is no buffer to read then this will just give us nullptr on buffp
			size_t buffSize = sockReads[i]->pop_latest_buff(buffp);

			if(buffp == nullptr) continue;

			std::string messageString = static_cast<char*>(buffp->data());
			//removes the newline at the end, partly because my ui cannot handle newlines...
			// messageString.pop_back();

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
					sockReads[i]->device_ID = randNum;
					ui.printo("New ID: "+std::to_string(randNum)+", With name: NULL\n");
					nameFutures.push_back(ui.getDeviceName());

				}else{

					sockReads[i]->device_ID = receivedID;
					sockReads[i]->async_write(&receivedID, sizeof(uint32_t)); // echo the Device ID that was received
				}
			}else{

				ui.printo("Main, Message: " + messageString + " from " + IDs.at(sockReads[i]->device_ID));
				//actually handle the message
				dataSet data = dataToFloat(messageString.c_str());
				if(static_cast<char*>(buffp->data())[0] == '+'){

					std::string message = "Manual Data Send from device \"" +
						IDs.at(sockReads[i]->device_ID) + "\":\\nTemperature: "
						+ std::to_string(data.temp) + " degrees C\\nPressure: "
						+ std::to_string(data.pressure) + " PSI\\nFlow Rate: "
						+ std::to_string(data.flow) + " GPM";

					multiSendText(curl, phones, message, apiKey, "CoolantMonitor");
				}else{
					dataMonitors[i].inturpretData(messageString.c_str());
				}
			}
			free_buffer(buffp);
		}

		//Check for ready notifications
		for(size_t i = 0; i < dataMonitors.size(); i++){
			if(dataMonitors[i].timeOfNotification < time(NULL) && dataMonitors[i].timeOfNotification != 0){

				std::string message = ":\\nTemperature: " +
					std::to_string(dataMonitors[i].previous.temp) + " degrees C\\nPressure: " +
					std::to_string(dataMonitors[i].previous.pressure) +
					" PSI\\nFlow Rate: " +
					std::to_string(dataMonitors[i].previous.flow) +
					" GPM";

				dataMonitors[i].timeOfNotification = 0;

				if(dataMonitors[i].machineIsOn){
					message = "Machine Turned On from device \"" + IDs.at(sockReads[i]->device_ID) + "\"" + message;
				}else{
					message = "Machine Turned Off from device \"" + IDs.at(sockReads[i]->device_ID) + "\""  + message;
				}

				multiSendText(curl, phones, message, apiKey, "CoolantMonitor");
			}
		}

		// resolve ready name futures
		bool updatedAName = false;
		for(auto futures_it = nameFutures.begin(); futures_it != nameFutures.end(); futures_it++){
			if(futures_it->wait_for(std::chrono::seconds(0)) == std::future_status::ready){
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

		std::string firstArg = "";
		for(size_t i = 0; !isspace(commandString[i]) && i < commandString.size(); i++){
			firstArg += commandString[i];
		}

		//handle that command
		if(commandString == "help"){
			ui.printc("help - print this commands list");
			ui.printc("rename <device id> <new name> - rename a device");
			ui.printc("list - list all devices and names");
		}
		else if(commandString == "list"){
			ui.printc("ID, Name");
			for(auto it = IDs.begin(); it != IDs.end(); it++){
				ui.printc(std::to_string(it->first) + ", " + it->second);
			}
		}
		else if(firstArg == "rename"){

			std::string id = "";

			for(size_t i = 7; !isspace(commandString[i]) && i < commandString.size(); i++){
				id += commandString[i];
			}
			unsigned int int_id = std::stoul(id);

			std::string new_name = "";

			for(size_t i = id.size()+8; i < commandString.size(); i++){
				new_name += commandString[i];
			}

			for(auto it = IDs.begin(); it != IDs.end(); it++){
				if(it->first == int_id){
					it->second = new_name;
				}
			}
		}

		if(commandString != ""){
			ui.printc("> " + commandString);
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

	ui.printoImmediate("Stopping mdns service, this may take a minute...\n");

	mdnsStopTh.join();

	curl_easy_cleanup(curl);

	return mainRet;
}
