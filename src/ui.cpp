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
	endwin();
}

void cursesUi::update(){

	if(!changedSinceUpdate) return;

	clear();
	wclear(outputWin);
	wclear(commandWin);

	int tmpcols = getmaxx(stdscr);
	int tmplines = getmaxy(stdscr);

	if(tmpcols != currentCols || tmplines != currentLines){
		resize_helper(tmpcols, tmplines);
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
	char * ourstr = new char[len];
	
	for(int i = 0; i < len; i++){
		ourstr[i] = str[i];
	}

	if(numOutputStrs == MAX_OUTPUT_STRINGS){

		for(int i = 512; i < MAX_OUTPUT_STRINGS; i++){

			delete outputStrs[i-512];
			outputStrs[i-512] = outputStrs[i];
			numOutputStrs = 512;
		}
	}
	outputStrs[numOutputStrs] = ourstr;
	numOutputStrs++;

	changedSinceUpdate = true;
}

void cursesUi::printo(char * && str){

	if(numOutputStrs == MAX_OUTPUT_STRINGS){

		for(int i = 512; i < MAX_OUTPUT_STRINGS; i++){

			delete outputStrs[i-512];
			outputStrs[i-512] = outputStrs[i];
			numOutputStrs = 512;
		}
	}
	outputStrs[numOutputStrs] = str;
	numOutputStrs++;
	
	changedSinceUpdate = true;
}

void cursesUi::printo(std::string str){

	printo(str.c_str());
}

void cursesUi::printo(std::string && str){

	printo(std::move(str.data()));
}

void cursesUi::resize_helper(int cols, int lines){

	mvwin(outputWin, 0, cols/2);
	wresize(commandWin, lines, (cols/2));
	wresize(outputWin, lines, (cols/2));
}
