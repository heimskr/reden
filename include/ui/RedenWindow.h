#include <gtkmm.h>

namespace Reden {
	class RedenWindow: public Gtk::ApplicationWindow {
		public:
			Gtk::HeaderBar *header;

			RedenWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

			static RedenWindow * create();

		private:
			Glib::RefPtr<Gtk::Builder> builder;
	};
}
