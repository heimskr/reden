#pragma once

#include <gtkmm.h>

#include "ui/RedenWindow.h"

namespace Reden {
	class App: public Gtk::Application {
		public:
			Glib::RefPtr<Gtk::Builder> builder;

			static Glib::RefPtr<App> create();
			static const char * getText(const std::string &path, gsize &);
			static const char * getText(const std::string &path);

			void on_startup() override;
			void on_activate() override;

			RedenWindow * create_window();

		protected:
			App();

		private:
			void on_hide_window(Gtk::Window *);	
	};
}
