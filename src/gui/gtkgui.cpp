#include "gtkgui.h"
#include <glibmm/dispatcher.h>
#include <glibmm/ustring.h>
#include <gtkmm/textbuffer.h>
#include <iostream>
#include <mutex>
#include <sigc++/functors/mem_fun.h>

void CGuiWindow::run(int argc , char** argv){

	m_app->run(argc, argv);
}

void CGuiWindow::onPrinto(){

	std::scoped_lock lock(m_memberMutex);
	if(m_strToPrint != nullptr){

		Glib::RefPtr<Gtk::TextBuffer> buff = m_textView.get_buffer();
		Gtk::TextBuffer::iterator iter = buff->get_iter_at_offset(buff->get_char_count());
		buff->insert(iter, m_strToPrint);
		// TODO: shorten the output if it gets too long
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
	m_window.set_default_size(200, 200);
	m_window.set_child(m_topLevelBox);

	m_topLevelBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_topLevelBox.append(m_outputBox);
	m_topLevelBox.append(m_commandBox);

	m_outputBox.set_margin(MARGIN_PX);
	m_outputBox.set_margin_bottom(0);
	m_outputBox.set_orientation(Gtk::Orientation::VERTICAL);
	m_outputBox.append(m_frame);

	m_frame.set_child(m_textView);

	m_textView.set_margin(0);
	m_textView.set_pixels_above_lines(3);
	m_textView.set_pixels_below_lines(3);
	m_textView.set_left_margin(3);
	m_textView.set_right_margin(3);
	m_textView.set_editable(false);
	m_textView.set_focusable(false);
	m_textView.set_wrap_mode(Gtk::WrapMode::WORD);
	m_textView.set_expand(true);

	m_commandBox.set_margin(MARGIN_PX);
	m_commandBox.append(m_commandEntry);
	m_commandBox.append(m_commandButton);

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

		if(m_strToPrint != nullptr){
			delete[] m_strToPrint;
			m_strToPrint = nullptr;
		}

		if(str != nullptr){
			int len = strlen(str);
			m_strToPrint = new char[len+1];
			
			for(int i = 0; i <= len; i++){
				m_strToPrint[i] = str[i];
			}
		}

		m_printoDispatcher.emit();
	}
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
