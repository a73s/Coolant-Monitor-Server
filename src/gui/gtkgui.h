#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm.h>
#include <gtkmm/entry.h>
#include <gtkmm/enums.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <sigc++/functors/mem_fun.h>
#include <mutex>

#include <future>

#define MARGIN_PX 5

class CGuiWindow
{
public:
	CGuiWindow();
	~CGuiWindow();

	void update(){}
	void run(int argc , char** argv);

	void printo(char const * const str);
	void printo(std::string str);
	void printoImmediate(char const * const str);
	void printoImmediate(std::string str);
	void printc(char const * const str);
	void printc(std::string str);
	std::future<std::string> getDeviceName();
	std::string getCommand();

private:
	void onPrinto();
	void onPrintc();
	void onSubmitButtonHit();
	void onEnterHit();

	std::mutex m_memberMutex;

	std::vector<char *> m_strsToPrintOutput;
	std::vector<char *> m_strsToPrintCommandOutput;

	std::vector<std::promise<std::string>> m_nameQueue;
	std::vector<std::string> m_commands;

	Glib::RefPtr<Gtk::Application> m_app = Gtk::Application::create("Coolant Monitor");
	Glib::Dispatcher m_printoDispatcher;
	Glib::Dispatcher m_printcDispatcher;

	Gtk::Window m_window;
	Gtk::HeaderBar m_titleBar;
	Gtk::Box m_halfSeperatorBox;
	Gtk::Box m_leftSideBox;
	Gtk::Box m_rightSideBox;
	Gtk::Box m_commandOutputBox;
	Gtk::Box m_outputBox;
	Gtk::ScrolledWindow m_outputScrolledWindow;
	Gtk::ScrolledWindow m_commandOutputScrolledWindow;
	Gtk::TextView m_outputTextView;
	Gtk::TextView m_commandOutputTextView;
	Gtk::Box m_commandInputBox;
	Gtk::Frame m_commandOutputFrame;
	Gtk::Frame m_outputFrame;
	Gtk::Button m_commandButton;
	Gtk::Entry m_commandEntry;

	Gtk::Dialog m_nameEntryWindow;
	Gtk::Entry m_nameEntry;
	Gtk::HeaderBar m_nameEntryBar;
};
