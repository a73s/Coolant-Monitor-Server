#include <cstring>
#include <ncurses.h>
#include <string>

#include "ui.h"

cursesUi::cursesUi(){

	initscr();
	noecho();
	currentCols = COLS;
	currentLines = LINES;

	commandWin = newwin(LINES, COLS/2, 0, 0);

	outputWin = newwin(LINES, COLS/2, 0, COLS/2 );

	changedSinceUpdate = true;
	update();
}

cursesUi::~cursesUi(){
	for(int i = 0; i < numOutputStrs; i++){
		delete[] outputStrs[i];
	}
	numOutputStrs = 0;
	endwin();
}

void cursesUi::update(){

	// must call refresh first to update that values that getmax functions return
	refresh();
	int tmpcols = getmaxx(stdscr);
	int tmplines = getmaxy(stdscr);

	if(tmpcols != currentCols || tmplines != currentLines){
		resize_helper(tmpcols, tmplines);
	}
	
	if(!changedSinceUpdate) return;

	clear();
	wclear(outputWin);
	wclear(commandWin);
	
	int printlinenum = 1;
	int i = numOutputStrs-1;

	while(i >= 0 && printlinenum < currentLines-1){

		mvwprintw(outputWin, printlinenum, 1, "%s", outputStrs[i]);
		i--;
		printlinenum++;
	}

	box(commandWin, 0, 0);
	box(outputWin, 0, 0);
	mvwprintw(commandWin, 0, 1, "Command");
	mvwprintw(outputWin, 0, 1, "Output");

	refresh();
	wrefresh(outputWin);
	wrefresh(commandWin);

	changedSinceUpdate = false;
}

void cursesUi::printo(char const * const str){

	int len = strlen(str);
	char * ourstr = new char[len+1];
	
	for(int i = 0; i <= len; i++){
		ourstr[i] = str[i];
	}

	constexpr int halfMax = MAX_OUTPUT_STRINGS/2;

	// if full, delete half the strings
	if(numOutputStrs == MAX_OUTPUT_STRINGS){

		for(int i = halfMax; i < MAX_OUTPUT_STRINGS; i++){

			delete[] outputStrs[i-halfMax];
			outputStrs[i-halfMax] = outputStrs[i];
			numOutputStrs--;
		}
	}
	outputStrs[numOutputStrs] = ourstr;
	numOutputStrs++;

	changedSinceUpdate = true;
}

void cursesUi::printo(std::string str){

	printo(str.c_str());
}

void cursesUi::printoImmediate(char const * const str){

	printo(str);
	update();
}
void cursesUi::printoImmediate(std::string str){

	printo(str);
	update();
}

void cursesUi::resize_helper(int cols, int lines){

	wresize(commandWin, lines, (cols/2));
	wresize(outputWin, lines, (cols/2));
	mvwin(outputWin, 0, cols/2);
	currentLines = lines;
	currentCols = cols;
	changedSinceUpdate = true;
}
