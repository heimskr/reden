#pragma once

#include <gtkmm.h>

#include "ui/BasicEntry.h"

namespace Reden {
	class ConnectDialog: public Gtk::Dialog {
		public:
			ConnectDialog(const Glib::ustring &title, Gtk::Window &parent, bool modal = true);

			sigc::signal<void(const Glib::ustring &, const Glib::ustring &, const Glib::ustring &)>
			signal_submit() const { return signal_submit_; }

		private:
			BasicEntry hostEntry, portEntry, nickEntry;
			Gtk::Box buttonBox {Gtk::Orientation::HORIZONTAL};
			Gtk::Button cancelButton {"Cancel"}, okButton {"OK"};
			sigc::signal<void(const Glib::ustring &, const Glib::ustring &, const Glib::ustring &)> signal_submit_;

			void on_activate();
			void submit();
	};
}
