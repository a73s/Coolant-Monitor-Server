#pragma once

#define MACHINE_ON_FLOW_TRIGGER 1
#define MACHINE_OFF_FLOW_TRIGGER 0.5
#define NOTIFICATION_TIME_DELAY 60

struct dataSet{
	float temp = 0.0;
	float pressure = 0.0;
	float flow = 0.0;
};

dataSet dataToFloat(const char * str);

struct dataMonitor{

	bool machineIsOn = false;
	bool isFirstData = true;

	dataSet previous = {0.0,0.0,0.0};

	//indicates when a notification should be sent out, containing the prev_* data
	unsigned int timeOfNotification = 0;

	//sets the timeOfNotification and prev_* data
	//this should be run for every received data
	void inturpretData(const dataSet data);
	void inturpretData(const char * str){
		inturpretData(dataToFloat(str));
	}
};

