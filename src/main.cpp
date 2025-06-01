#include "gui/gtkgui.h"
#include "primary.h"
#include <thread>
#include <iostream>
#include <thread>

CGuiWindow ui;

int main(int argc, char* argv[]) {

	std::thread primaryThread(primary);
	ui.run(argc, argv);
	primaryThread.join();
	return 0;
}
