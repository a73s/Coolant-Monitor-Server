#include "gtkgui.h"
#include <glibmm/dispatcher.h>
#include <glibmm/ustring.h>
#include <gtkmm/enums.h>
#include <gtkmm/textbuffer.h>
#include <iostream>
#include <mutex>
#include <sigc++/functors/mem_fun.h>

void CGuiWindow::run(int argc , char** argv){

	m_app->run(argc, argv);
}

void CGuiWindow::onPrinto(){

	std::scoped_lock lock(m_memberMutex);

	if(m_strsToPrint.size()){

		Glib::RefPtr<Gtk::TextBuffer> buff = m_outputTextView.get_buffer();
		Gtk::TextBuffer::iterator iter = buff->get_iter_at_offset(0);
		char * toPrint = *m_strsToPrint.begin();
		m_strsToPrint.erase(m_strsToPrint.begin());
		buff->insert(iter, toPrint);
		// TODO: shorten the output if it gets too long

		delete[] toPrint;
	}
}

void CGuiWindow::onSubmitButtonHit(){
	std::cout << m_commandEntry.get_buffer()->get_text() << std::endl;
}

void CGuiWindow::onEnterHit(){
	std::cout << m_commandEntry.get_buffer()->get_text() << std::endl;
}

CGuiWindow::CGuiWindow()
{
	m_window.set_titlebar(m_titleBar);
	m_window.set_title("Coolant Monitor Server");
	m_window.set_default_size(900, 600);
	m_window.set_child(m_halfSeperatorBox);

	m_halfSeperatorBox.set_orientation(Gtk::Orientation::HORIZONTAL);
	m_halfSeperatorBox.append(m_leftSideBox);
	m_halfSeperatorBox.append(m_rightSideBox);
	m_halfSeperatorBox.set_homogeneous();

	m_leftSideBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_leftSideBox.append(m_commandOutputBox);
	m_leftSideBox.append(m_commandInputBox);

	m_rightSideBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_rightSideBox.append(m_outputBox);

	m_outputBox.set_margin(MARGIN_PX);
	m_outputBox.set_margin_start(0);
	m_outputBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_outputBox.append(m_outputFrame);

	m_commandOutputBox.set_margin(MARGIN_PX);
	m_commandOutputBox.set_margin_bottom(0);
	m_commandOutputBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_commandOutputBox.append(m_commandOutputFrame);

	m_commandOutputFrame.set_child(m_commandOutputScrolledWindow);

	m_commandOutputScrolledWindow.set_child(m_commandOutputTextView);

	m_outputFrame.set_child(m_outputScrolledWindow);

	m_outputScrolledWindow.set_child(m_outputTextView);

	m_outputTextView.set_margin(0);
	m_outputTextView.set_pixels_above_lines(3);
	m_outputTextView.set_pixels_below_lines(3);
	m_outputTextView.set_left_margin(3);
	m_outputTextView.set_right_margin(3);
	m_outputTextView.set_editable(false);
	m_outputTextView.set_focusable(false);
	m_outputTextView.set_wrap_mode(Gtk::WrapMode::WORD);
	m_outputTextView.set_expand(true);

	m_commandOutputTextView.set_margin(0);
	m_commandOutputTextView.set_pixels_above_lines(3);
	m_commandOutputTextView.set_pixels_below_lines(3);
	m_commandOutputTextView.set_left_margin(3);
	m_commandOutputTextView.set_right_margin(3);
	m_commandOutputTextView.set_editable(false);
	m_commandOutputTextView.set_focusable(false);
	m_commandOutputTextView.set_wrap_mode(Gtk::WrapMode::WORD);
	m_commandOutputTextView.set_expand(true);

	m_commandInputBox.set_margin(MARGIN_PX);
	m_commandInputBox.append(m_commandEntry);
	m_commandInputBox.append(m_commandButton);

	m_commandEntry.set_hexpand(true);
	m_commandEntry.signal_activate().connect(sigc::mem_fun(*this, &CGuiWindow::onEnterHit));

	m_commandButton.set_label("Submit Command");
	m_commandButton.set_margin_start(MARGIN_PX);
	m_commandButton.set_can_focus(false);
	m_commandButton.signal_clicked().connect(sigc::mem_fun(*this, &CGuiWindow::onSubmitButtonHit));

	m_printoDispatcher.connect(sigc::mem_fun(*this, &CGuiWindow::onPrinto));

	m_app->signal_activate().connect([&](){
		m_app->add_window(m_window);
		m_window.show();
	});
}

void CGuiWindow::printo(char const * const str){

	{
		std::scoped_lock lock(m_memberMutex);

		if(str != nullptr){
			int len = strlen(str);
			char * tmpstr = new char[len+1];
			
			for(int i = 0; i <= len; i++){
				tmpstr[i] = str[i];
			}
			m_strsToPrint.push_back(tmpstr);
		}
	}

	m_printoDispatcher.emit();
}

void CGuiWindow::printo(std::string str){

	printo(str.c_str());
}

void CGuiWindow::printoImmediate(char const * const str){

	printo(str);
}
void CGuiWindow::printoImmediate(std::string str){

	printo(str);
}

void CGuiWindow::printc(char const * const str){
	//TODO:
}

void CGuiWindow::printc(std::string str){
	printc(str.c_str());
}

std::future<std::string> CGuiWindow::getDeviceName(){

	//TODO:
	std::promise<std::string> namePromise;
	std::future<std::string> tmpFut = namePromise.get_future();
	namePromise.set_value("CUM");
	return tmpFut;
}

std::string CGuiWindow::getCommand(){

	//TODO:
	return "command";
}
