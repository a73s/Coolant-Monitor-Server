#pragma once

#include <ncurses.h>
// #include <queue>
#include <string>
#include <thread>

constexpr int MAX_OUTPUT_STRINGS = 1024;

class cursesUi{

public:
	cursesUi();

	void update();

	void printo(char const * const str);
	void printo(char * && str);
	void printo(std::string str);
	void printo(std::string && str);
	
	~cursesUi();

private:

	void resize_helper(int cols, int lines);

	std::thread watcherThread;
	WINDOW * commandWin = nullptr;
	WINDOW * outputWin = nullptr;

	int currentCols = 0;
	int currentLines = 0;

	bool changedSinceUpdate = false;
	char * outputStrs[MAX_OUTPUT_STRINGS];
	int numOutputStrs = 0;
};
