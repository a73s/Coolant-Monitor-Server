#pragma once

#include <future>
#include <ncurses.h>
#include <string>
#include <thread>
#include <vector>

constexpr int MAX_OUTPUT_STRINGS = 1024;
constexpr int NAME_INPUT_BOX_WIDTH = 35;
constexpr int NAME_INPUT_BOX_HEIGHT = 3;
constexpr int MAX_NAME_LEN = NAME_INPUT_BOX_WIDTH - 3;

class cursesUi{

public:
	cursesUi();

	void update();

	void printo(char const * const str);
	void printo(std::string str);
	void printoImmediate(char const * const str);
	void printoImmediate(std::string str);
	void printc(char const * const str);
	void printc(std::string str);
	std::future<std::string> getDeviceName();
	std::string getCommand();
	
	~cursesUi();

private:

	void resize_helper(int cols, int lines);

	std::thread watcherThread;
	WINDOW * commandWin = nullptr;
	WINDOW * outputWin = nullptr;
	WINDOW * nameInputWin = nullptr;

	int currentCols = 0;
	int currentLines = 0;

	bool changedSinceUpdate = false;

	char * outputStrs[MAX_OUTPUT_STRINGS];
	int numOutputStrs = 0;

	char * commandOutputStrs[MAX_OUTPUT_STRINGS];
	int numCommandOutputStrs = 0;

	std::vector<std::promise<std::string>> nameQueue;
	std::vector<std::string> commands;
	std::string namestr;
	std::string cmdstr;
};
