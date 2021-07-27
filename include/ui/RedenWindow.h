#pragma once

#include <gtkmm.h>
#include <map>

#include "ui/MainBox.h"
#include "Connection.h"

namespace Reden {
	class RedenWindow: public Gtk::ApplicationWindow {
		public:
			Gtk::HeaderBar *header;

			RedenWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

			static RedenWindow * create();

		private:
			Glib::RefPtr<Gtk::Builder> builder;
			Glib::RefPtr<Gtk::CssProvider> cssProvider;
			ConnectionMap connections;
			MainBox mainBox;
	};
}
