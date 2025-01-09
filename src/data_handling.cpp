#include "data_handling.h"

#include <ctime>
#include <string>

void dataMonitor::inturpretData(const dataSet data){

	isFirstData = false;

	if(!machineIsOn && data.flow > MACHINE_ON_FLOW_TRIGGER){

		machineIsOn = true;
		timeOfNotification = time(NULL) + NOTIFICATION_TIME_DELAY;

	}else if(machineIsOn && data.flow < MACHINE_OFF_FLOW_TRIGGER){

		machineIsOn = false;
		timeOfNotification = time(NULL) + NOTIFICATION_TIME_DELAY;
	}

	previous.flow = data.flow;
	previous.pressure = data.pressure;
	previous.temp = data.temp;

	return;
}

dataSet dataToFloat(const char * str){

	int state = 0;
	int runningIndex = 0;
	std::string temp;
	std::string pressure;
	std::string flow;

	while(str[runningIndex] != '\n' && str[runningIndex] != '\0'){

		runningIndex++;

		if(str[runningIndex] == ','){
			state++;
			continue;
		}

		switch(state){
			case 0:{

				temp += str[runningIndex];
				break;
			}
			case 1:{

				pressure += str[runningIndex];
				break;
			}
			case 2:{

				flow += str[runningIndex];
				break;
			}
		}
	}

	return {std::stof(temp),std::stof(pressure),std::stof(flow)};
}
