#include <glibmm/refptr.h>
#include <glibmm/ustring.h>
#include <gtkmm.h>
#include <gtkmm/entry.h>
#include <gtkmm/enums.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <sigc++/functors/mem_fun.h>
#include <mutex>

#include <future>

#define MARGIN_PX 4

class CGuiWindow
{
public:
	CGuiWindow();

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

	void onPrinto();

private:
	void onSubmitButtonHit();
	void onEnterHit();
	Glib::Dispatcher m_printoDispatcher;
	Glib::RefPtr<Gtk::Application> m_app = Gtk::Application::create("Coolant Monitor");
	std::vector<char *> m_strsToPrint;

	std::mutex m_memberMutex;

	Gtk::Window m_window;
	Gtk::HeaderBar m_titleBar;
	Gtk::Box m_topLevelBox;
	Gtk::Box m_outputBox;
	Gtk::Box m_commandBox;
	Gtk::Frame m_frame;
	Gtk::TextView m_textView;
	Gtk::Button m_commandButton;
	Gtk::Entry m_commandEntry;
};
