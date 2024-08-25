#include <cassert>
#include <cstring>
#include <ncurses.h>
#include <string>
#include <cstring>

#include "ui.h"

cursesUi::cursesUi(){

	initscr();
	noecho();
	cbreak();
	nodelay(stdscr, true);
	currentCols = COLS;
	currentLines = LINES;

	commandWin = newwin(LINES, COLS/2, 0, 0);
	outputWin = newwin(LINES, COLS/2, 0, COLS/2 );
	nameInputWin = newwin(NAME_INPUT_BOX_HEIGHT, NAME_INPUT_BOX_WIDTH, (LINES/2)-(NAME_INPUT_BOX_HEIGHT/2), (COLS/2)-(NAME_INPUT_BOX_WIDTH/2));

	changedSinceUpdate = true;
	update();
}

cursesUi::~cursesUi(){
	for(int i = 0; i < numOutputStrs; i++){
		delete[] outputStrs[i];
	}
	for(int i = 0; i < numCommandOutputStrs; i++){
		delete[] commandOutputStrs[i];
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

	char newchar = -1;

	// set between command input and name input
	std::string * inputstr = &namestr;
	if(!nameQueue.size()){
		inputstr = &cmdstr;
	}

	// handle inputs
	newchar = getch();
	while(newchar != ERR){
		changedSinceUpdate = true;

		switch(newchar){
			case 127:
			case '\b':{
				if(inputstr->size()){
					inputstr->pop_back();
				}
				break;
			}
			case '\n':{
				if(inputstr == &namestr){
					if(namestr != ""){
						nameQueue.front().set_value(namestr);
						nameQueue.erase(nameQueue.begin());
					}
				}
				if(inputstr == &cmdstr){
					if(cmdstr != ""){
						commands.push_back(cmdstr);
					}
				}
				*inputstr = "";
				break;
			}
			default:{
				if((inputstr->size() == MAX_NAME_LEN && inputstr == &namestr)){
					break;
				}else if(isprint(newchar)){
					*inputstr += newchar;
				}
				break;
			}
		}
		newchar = getch();
	}

	assert(newchar == ERR);

	if(!changedSinceUpdate) return;

	clear();
	wclear(outputWin);
	wclear(commandWin);
	wclear(nameInputWin);
	
	int printlinenum = 1;
	int i = numOutputStrs-1;

	while(i >= 0 && printlinenum < currentLines-1){

		mvwprintw(outputWin, printlinenum, 1, "%s", outputStrs[i]);
		i--;
		printlinenum++;
	}

	printlinenum = 1;
	i = numCommandOutputStrs-1;
	while(i >= 0 && printlinenum < currentLines-3){

		mvwprintw(commandWin, printlinenum, 1, "%s", commandOutputStrs[i]);
		i--;
		printlinenum++;
	}

	box(commandWin, 0, 0);
	box(outputWin, 0, 0);
	mvwprintw(commandWin, 0, 1, "Command");
	mvwprintw(outputWin, 0, 1, "Output");

	wmove(commandWin, currentLines - 3, 1);
	whline(commandWin, ACS_HLINE, (currentCols/2) - 2);

	mvwprintw(commandWin, currentLines-2, 1, "%s", cmdstr.c_str());

	if(nameQueue.size()){
		box(nameInputWin, 0, 0);
		mvwprintw(nameInputWin, 0, 1, "Device Name Input");
		mvwprintw(nameInputWin, 1, 1, "%s", namestr.c_str());
		move((currentLines/2)-(NAME_INPUT_BOX_HEIGHT/2)+1, (currentCols/2)-(NAME_INPUT_BOX_WIDTH/2)+1+namestr.size());
	}else{
		move(currentLines-2, cmdstr.size()+1);
	}

	refresh();
	wnoutrefresh(outputWin);
	wnoutrefresh(commandWin);
	if(nameQueue.size()) wnoutrefresh(nameInputWin);
	doupdate();

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

void cursesUi::printc(char const * const str){

	int len = strlen(str);
	char * ourstr = new char[len+1];
	
	for(int i = 0; i <= len; i++){
		ourstr[i] = str[i];
	}

	constexpr int halfMax = MAX_OUTPUT_STRINGS/2;

	// if full, delete half the strings
	if(numCommandOutputStrs == MAX_OUTPUT_STRINGS){

		for(int i = halfMax; i < MAX_OUTPUT_STRINGS; i++){

			delete[] commandOutputStrs[i-halfMax];
			commandOutputStrs[i-halfMax] = commandOutputStrs[i];
			numCommandOutputStrs--;
		}
	}
	commandOutputStrs[numCommandOutputStrs] = ourstr;
	numCommandOutputStrs++;

	changedSinceUpdate = true;
}

void cursesUi::printc(std::string str){
	printc(str.c_str());
}

std::future<std::string> cursesUi::getDeviceName(){

	std::promise<std::string> namePromise;
	std::future<std::string> tmpFut = namePromise.get_future();
	nameQueue.push_back(std::move(namePromise));
	changedSinceUpdate = true;

	return tmpFut;
}

std::string cursesUi::getCommand(){

	if(commands.size()){
		std::string tmp = std::move(commands.front());
		commands.erase(commands.begin());
		return tmp;
	}

	return "";
}

void cursesUi::resize_helper(int cols, int lines){

	wresize(commandWin, lines, (cols/2));
	wresize(outputWin, lines, (cols/2));
	wresize(nameInputWin, NAME_INPUT_BOX_HEIGHT, NAME_INPUT_BOX_WIDTH);
	mvwin(outputWin, 0, cols/2);
	mvwin(nameInputWin, (lines/2)-(NAME_INPUT_BOX_HEIGHT/2), (cols/2)-(NAME_INPUT_BOX_WIDTH/2));
	currentLines = lines;
	currentCols = cols;
	changedSinceUpdate = true;
}
